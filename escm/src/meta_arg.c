/************************************************************************
 * meta_arg.c - another implementation of the second line meta-argument
 *              processing but NOT compatible.
 * $Id$
 ************************************************************************
 *
 * Copyright (c) 2002-2003 TAGA Yoshitaka, All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
/* Usage:
 * n = meta_args(&argc, &argv);
 *
 * Example:
 *  $ foo bar
 *  and "foo" begins with:
 *  #!/usr/local/bin/interpreter \
 *    -e '(define x "abc")' \
 *    -s
 * =>
 *  argv[0] = "/usr/local/bin/interpreter";
 *  argv[1] = "-e";
 *  argv[2] = "(define x \"abc\")"
 *  argv[3] = "-s";
 *  argv[4] = "foo";
 *  argv[5] = "bar";
 *  argv[6] = NULL;
 *
 * Return the number of meta-arguments (a non-negative).
 *
 * This is another implementation of the second-line meta-argument
 * processing introduced by scsh but is NOT compatible with it
 * (see Defferences below).
 * 
 * Differences from scsh's meta-argument processing:
 * - It parses the meta-argument lines as shells do, but does not expand
 *   wild characters. It means you can use tabs, double quotation marks
 *   and single quotation marks.
 * - The meta-arguments can be stored in successing lines.
 *   Put a backslash at the end of line to continue.
 * - The ANSI C escapes (\r, \n, etc.) are not supported.
 *
 * Macros and their behaviors:
 * - xmalloc(size)            malloc(size)
 * - xrealloc(ptr, size)      realloc(ptr, size)
 * - xerror0()                print an error message and exit            
 * - xerror1(msg)
 * - xerror2(fmt, arg)
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include "meta_arg.h"

#ifndef FALSE
# define FALSE 0
#endif /* FALSE */
#ifndef TRUE
# define TRUE !FALSE
#endif /* TRUE */

#ifndef xprog
static char *xprog = NULL;
#endif /* xprog */
#ifndef xfile
static char *xfile = NULL;
#endif /* xfile */
#ifndef xlineno
static int xlineno = 0;
#endif /* xlineno */

#ifndef xmalloc
# define xmalloc(size) malloc((size))
#endif /* ndef xmalloc */
#ifndef xrealloc
# define xrealloc(ptr, size) realloc((ptr), (size))
#endif /* ndef xrealloc */
#ifndef xerror0
# define xerror0() {\
  perror(xprog);\
  exit(EXIT_FAILURE);\
}
#endif /* xerror0 */
#ifndef xerror1
# define xerror1(str) {\
  fprintf(stderr, "%s: ", xprog);\
  if (xfile) fprintf(stderr, "%s: ", xfile);\
  if (xlineno) fprintf(stderr, "%d: ", xlineno);\
  fputs(str, stderr);\
  exit(EXIT_FAILURE);\
}
#endif /* xerror1 */
#ifndef xerror2
# define xerror2(fmt, arg) {\
  fprintf(stderr, "%s: ", xprog);\
  if (xfile) fprintf(stderr, "%s: ", xfile);\
  if (xlineno) fprintf(stderr, "%d: ", xlineno);\
  fprintf(stderr, fmt, arg);\
  exit(EXIT_FAILURE);\
}
#endif /* xerror2 */

/* meta_progname(argv[0]) - return the program name.
 */
const char *
meta_progname(const char *argv0)
{
  const char *p = argv0;
  while (*p)
    p++;
  while (argv0 < p && *p != '/')
    p--;
  if (p == argv0 || !p[1]) return argv0;
  else return p + 1;
}


#define ADD_CHAR(c) {\
  if (buf) {\
     if (size == i) {\
        size += BUFSIZ;\
        buf = (char *)xrealloc(buf, size);\
        if (!buf) xerror0();\
     }\
     buf[i++] = (char)c;\
  }\
}
/* skip_shebang_line(fp) - skip the sharp-bang line.
 * Return META_ARGS_NOT          if it does not have a meta-argument switch;
 *        META_ARGS_OK           if it has a meta-argument switch.
 */
static int
skip_shebang_line(FILE *fp)
{
  int c;

  xlineno = 1;
  c = getc(fp);
  if (c == EOF) return META_ARGS_NOT;
  else if (c != '#') {
    ungetc(c, fp);
    return META_ARGS_NOT;
  }
  c = getc(fp);
  if (c != '!') xerror1("not a script file");
  for (;;) { /* skip blanks if any */
    c = getc(fp);
    if (c == EOF || c == '\n') xerror1("not a script file");
    if (c != ' ' && c != '\t') break;
  }
  for (;;) { /* skip argv[0] */
    c = getc(fp);
    if (c == EOF || c == '\n') return META_ARGS_NOT;
    if (c == ' ' || c == '\t') break;
  }
  for (;;) { /* skip blanks */
    c = getc(fp);
    if (c == EOF || c == '\n') return META_ARGS_NOT;
    if (c != ' ' && c != '\t') break;
  }
  if (c == '\\') {
    c = getc(fp);
    if (c == EOF) return META_ARGS_NOT;
    if (c == '\n') return META_ARGS_OK;
  }
  for (;;) {
    c = getc(fp);
    if (c == EOF || c == '\n') break;
  }
  return META_ARGS_NOT;
}
/* parse_as_command_line(&buf, &size, fp) - tokenize meta-argument lines
 * as if they were specified from a terminal.
 * Return the number of meta-arguments or META_ARGS_SYNTAX_ERROR.
 */
static int
parse_as_command_line(char **pbuf, size_t *psize, FILE *fp)
{
  int i = 0;
  size_t size = *psize;
  int c;
  int n = 0;
  char *buf;
  enum {
    OUTSIDE,
    INSIDE,
    SINGLE,
    DOUBLE,
  } state = OUTSIDE;
  int keep_lineno = 0;

  xlineno = 2;
  buf = *pbuf;
  while ((c = getc(fp)) != EOF) {
    if (state == OUTSIDE) {
      if (c == '\"') {
	keep_lineno = xlineno;
	state = DOUBLE;
	n++;
      } else if (c == '\'') {
	keep_lineno = xlineno;
	state = SINGLE;
	n++;
      } else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) xerror1("unexpected eof");
	else if (c == '\n') {
	  xlineno++;
	  break;
	}
	state = INSIDE;
	n++;
	ADD_CHAR(c);
      } else if (c == '\n') {
	xlineno++;
	break;
      } else if (c == ' ' || c == '\t') continue;
      else {
	state = INSIDE;
	n++;
	ADD_CHAR(c);
      }
    } else if (state == INSIDE) {
      if (c == '\"') state = DOUBLE;
      else if (c == '\'') state = SINGLE;
      else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) xerror1("unexpected eof");
	else if (c == '\n') {
	  xlineno++;
	  continue;
	} else ADD_CHAR(c);
      } else if (c == '\n') {
	xlineno++;
	ADD_CHAR('\0');
	state = OUTSIDE;
	break;
      } else if (c == ' ' || c == '\t') {
	ADD_CHAR('\0');
	state = OUTSIDE;
      } else ADD_CHAR(c);
    } else if (state == DOUBLE) {
      if (c == '\"') state = INSIDE;
      else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) xerror1("unexpected eof");
	else if (c == '\n') {
	  xlineno++;
	  continue;
	}
	ADD_CHAR(c);
      } else {
	if (c == '\n') xlineno++;
	ADD_CHAR(c);
      }
    } else { /* state == SINGLE */
      if (c == '\'') state = INSIDE;
      else {
	if (c == '\n') xlineno++;
	ADD_CHAR(c);
      }
    }
  }
  if (state == DOUBLE || state == SINGLE) {
    xlineno = keep_lineno;
    xerror1("unterminated string");
  } else if (state == INSIDE) xerror1("unexpected eof");
  *pbuf = buf;
  *psize = i;
  return n;
}
/* next_token(ptr, &size) - return the next token.
 */
static char *
next_token(char *ptr, size_t *psize)
{
  if (*psize == 0) return NULL;
  while (*ptr) {
    ptr++;
    (*psize)--;
  }
  ptr++;
  (*psize)--;
  return ptr;
}

int
meta_args_replace(int *pargc, char ***pargv, const char *script, int from)
{
  char **argv, **new_argv;
  char *buf, *ptr;
  int argc, ret, i, j;
  FILE *fp;
  size_t size = BUFSIZ;

  argc = *pargc;
  argv = *pargv;

  if (!xprog) xprog = meta_progname(argv[0]);

  fp = fopen(script, "r");
  if (fp == NULL) xerror2("can't open - %s", script);

  xfile = script;
  ret = skip_shebang_line(fp);
  if (ret == META_ARGS_OK) {
    buf = (char *)xmalloc(BUFSIZ);
    if (buf == NULL) xerror0();
    ret = parse_as_command_line(&buf, &size, fp);
  }
  fclose(fp);
  if (ret == META_ARGS_NOT) ret = 0;
  argc += 2 + ret - from;
  new_argv = (char **)xmalloc((sizeof(char*)) * (argc + 1));
  if (new_argv == NULL) xerror0();
  new_argv[0] = argv[0];
  if (ret > 0) {
    ptr = buf;
    for (i = 1; i <= ret; i++) {
      new_argv[i] = ptr;
      ptr = next_token(ptr, &size);
    }
  } else {
    i = 1;
  }
  new_argv[i++] = (char *)script;
  for (j = from; i <= argc; i++, j++) {
    new_argv[i] = argv[j];
  }
  *pargc = argc;
  *pargv = new_argv;
  return ret;
}
int
meta_args(int *pargc, char ***pargv)
{
  if (!(*pargc >= 3 && (*pargv)[1][0] == '\\' && (*pargv)[1][1] == '\0'))
    return META_ARGS_NOT;

  return meta_args_replace(pargc, pargv, (*pargv)[2], 3);
}

/* meta_skip_shebang(fp) - skip the sharp-bang line and meta-argument
 * lines if any.
 */
int
meta_skip_shebang(FILE *fp)
{
  int ret;
  char *buf = NULL;
  size_t size = 0;

  ret = skip_shebang_line(fp);
  if (ret != META_ARGS_OK) return ret;
  else return parse_as_command_line(&buf, &size, fp);
}

/*====================================================
 * the main function for debugging.
 *===================================================*/
#ifdef META_ARGS_TEST
int main(int argc, char **argv)
{
  int i;
  int ret;

  ret = meta_args(&argc, &argv);
  printf("meta-args: %d\n", ret);
  for (i = 0; i < argc; i++) {
    printf("%02d => >>%s<<\n", i, argv[i]);
  }
  return 0;
}
#endif /* def META_ARGS_TEST */
/* end of meta_arg.c */
