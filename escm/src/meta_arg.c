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
 * Return the number of meta-arguments (a non-negative),
 *         META_ARGS_NOT
 *         META_ARGS_ERRNO_ERROR,
 *         META_ARGS_SYNTAX_ERROR,
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
 * - xmalloc(size):            malloc(size);
 * - xrealloc(ptr, size):      realloc(ptr, size);
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include "meta_arg.h"

#ifndef FALSE
# define FALSE 0
#endif /* FALSE */
#ifndef TRUE
# define TRUE !FALSE
#endif /* TRUE */

#ifndef xmalloc
# define xmalloc(size) malloc((size))
#endif /* ndef xmalloc */
#ifndef xrealloc
# define xrealloc(ptr, size) realloc((ptr), (size))
#endif /* ndef xrealloc */

#define ADD_CHAR(c) {\
  if (buf) {\
     if (size == i) {\
        size += BUFSIZ;\
        buf = (char *)xrealloc(buf, size);\
        if (!buf) return META_ARGS_ERRNO_ERROR;\
     }\
     buf[i++] = (char)c;\
  }\
}
/* skip_shebang_line(fp) - skip the sharp-bang line.
 * Return META_ARGS_SYNTAX_ERROR if there is an error;
 *        META_ARGS_NOT          if it does not have a meta-argument switch;
 *        META_ARGS_OK           if it has a meta-argument switch.
 */
static int
skip_shebang_line(FILE *fp)
{
  int c;

  c = getc(fp);
  if (c == EOF) return META_ARGS_NOT;
  else if (c != '#') {
    ungetc(c, fp);
    return META_ARGS_NOT;
  }
  c = getc(fp);
  if (c != '!') return META_ARGS_SYNTAX_ERROR;
  for (;;) { /* skip blanks if any */
    c = getc(fp);
    if (c == EOF || c == '\n') return META_ARGS_SYNTAX_ERROR;
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

  buf = *pbuf;
  while ((c = getc(fp)) != EOF) {
    if (state == OUTSIDE) {
      if (c == '\"') {
	state = DOUBLE;
	n++;
      } else if (c == '\'') {
	state = SINGLE;
	n++;
      } else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) return META_ARGS_SYNTAX_ERROR;
	else if (c == '\n') break;
	state = INSIDE;
	n++;
	ADD_CHAR(c);
      } else if (c == '\n') break;
      else if (c == ' ' || c == '\t') continue;
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
	if (c == EOF) return META_ARGS_SYNTAX_ERROR;
	else if (c == '\n') continue;
	else ADD_CHAR(c);
      } else if (c == '\n') {
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
	if (c == EOF) return META_ARGS_SYNTAX_ERROR;
	else if (c == '\n') continue;
	ADD_CHAR(c);
      } else ADD_CHAR(c);
    } else { /* state == SINGLE */
      if (c == '\'') state = INSIDE;
      else ADD_CHAR(c);
    }
  }
  if (state != OUTSIDE) return META_ARGS_SYNTAX_ERROR;
  *pbuf = buf;
  *psize = i;
  return n;
}
/* next_token(ptr, &size) - return the next token.
 */
static char *
next_token(char *ptr, size_t *psize)
{
  printf("DEBUG: %s\n", ptr);
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
meta_args(int *pargc, char ***pargv)
{
  char **argv, **new_argv;
  char *buf, *ptr;
  int argc, ret, i;
  FILE *fp;
  size_t size = BUFSIZ;

  argc = *pargc;
  argv = *pargv;
  if (!(argc >= 3 && argv[1][0] == '\\' && argv[1][1] == '\0')) {
    return META_ARGS_NOT;
  }
  fp = fopen(argv[2], "r");
  if (fp == NULL) return META_ARGS_ERRNO_ERROR;
  ret = skip_shebang_line(fp);
  if (ret != META_ARGS_OK) {
    fclose(fp);
    return ret;
  }
  buf = (char *)xmalloc(BUFSIZ);
  if (buf == NULL) return META_ARGS_ERRNO_ERROR;
  ret = parse_as_command_line(&buf, &size, fp);
  if (ret < 0) return META_ARGS_SYNTAX_ERROR;
  argc += ret - 1;
  new_argv = (char **)xmalloc((sizeof(char*)) * (argc + 1));
  if (new_argv == NULL) return META_ARGS_ERRNO_ERROR;
  new_argv[0] = argv[0];
  ptr = buf;
  for (i = 1; i <= ret; i++) {
    printf("debug: %s\n", ptr);
    new_argv[i] = ptr;
    ptr = next_token(ptr, &size);
  }
  for (/**/; i <= argc; i++) {
    new_argv[i] = argv[i - ret + 1];
  }
  *pargc = argc;
  *pargv = new_argv;
  return ret;
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
