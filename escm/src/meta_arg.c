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
 * n_of_meta_args = meta_args(&argc, &argv);
 * n_of_meta_args = meta_args_keep_open(&argc, &argv, &fp);
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
 * This is another implementation of the second-line meta-argument
 * processing introduced by scsh but is NOT compatible with it
 * (see Defferences below).
 * 
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
 * - meta_arg_error():         { perror(argv[0]); exit(EXIT_FAILURE); }
 * - meta_arg_syntax_error():  { errno = EINVAL; meta_arg_error(); }
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

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

#ifndef meta_arg_error
# define meta_arg_error() escm_error(PACKAGE, NULL);
#endif /* ndef meta_arg_error */
#ifndef meta_arg_syntax_error
# define meta_arg_syntax_error() escm_error(PACKAGE, "Syntax error in meta-args")
#endif /* ndef meta_arg_syntax_error */

/*=====================================================
 * meta_args(&argc, &argv) - parse the meta-argument lines.
 * Return the number of meta-arguments if the script file uses the
 * meta-argument functionality, otherwise -1.
 *====================================================*/
#define META_ARG_ADDC(ch) {\
  if (i >= size) {\
    size += BUFSIZ;\
    buf = xrealloc(buf, size);\
    if (buf == NULL) meta_arg_error();\
  }\
  buf[i] = (char)(ch);\
  i++;\
}

enum META_TYPE {
  NOT_SHEBANG,
  NOT_META,
  META,
  EOF_REACHED,
};

static int
skip_to_eol(FILE *fp)
{
  int c;

  do {
    c = getc(fp);
    if (c == EOF) return FALSE;
  }while (c != '\n');
  return TRUE;
}

static enum META_TYPE
skip_meta_shebang_line(FILE *fp)
{
  int c;

  c = getc(fp);
  if (c != '#') {
    if (c != EOF) ungetc(c, fp);
    return NOT_SHEBANG;
  }
  c = getc(fp);
  if (c == EOF) return EOF_REACHED;
  else if (c == '\n') return NOT_META;
  else if (c != '!') {
    if (skip_to_eol(fp)) return NOT_META;
    else return EOF_REACHED;
  }
  do {
    c = getc(fp);
    if (c == EOF) return EOF_REACHED;
    if (c == '\n') return NOT_META;
  }while (c == ' ' || c == '\t');
  do {
    c = getc(fp);
    if (c == EOF) return EOF_REACHED;
    if (c == '\n') return NOT_META;
  }while (c != ' ' && c != '\t');
  do {
    c = getc(fp);
    if (c == EOF) return EOF_REACHED;
    if (c == '\n') return NOT_META;
  }while (c == ' ' || c == '\t');
  if (c == '\\') {
    c = getc(fp);
    if (c == EOF) return EOF_REACHED;
    else if (c == '\n') return META;
  }
  if (skip_to_eol(fp)) return NOT_META;
  else return EOF_REACHED;
}

int
meta_args(int *pargc, char ***pargv)
{
  FILE *fp;
  int argc, c, i, j, k, n_meta_args;
  char **new_argv, **argv, *buf;
  size_t size = BUFSIZ;
  enum {
    OUTSIDE,
    INSIDE,
    SINGLE_QUOTED,
    DOUBLE_QUOTED,
  } state = OUTSIDE;

  argc = *pargc;
  argv = *pargv;
  /* Check if we need to expand the meta-arguments. */
  if (!(argc >= 3 && argv[1][0] == '\\' && argv[1][1] == '\0')) {
    return -1;
  }

  /* Open the script file. */
  fp = fopen(argv[2], "r");
  if (fp == NULL) meta_arg_error();

  if (skip_meta_shebang_line(fp) != META) meta_arg_syntax_error();
 
  /* Parse the second line. */
  /* allocate the buffer. */
  buf = (char*)xmalloc(size);
  if (buf == NULL) meta_arg_error();

  /* parser. */
  i = 0;
  n_meta_args = 0;
  while ((c = getc(fp)) != EOF) {
    if (state == OUTSIDE) {
      if (c == ' ' || c == '\t') {
	continue;
      } else if (c == '\n') {
	break;
      } else if (c == '\'') {
	n_meta_args++;
	state = SINGLE_QUOTED;
      } else if (c == '\"') {
	n_meta_args++;
	state = DOUBLE_QUOTED;
      } else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) {
	  meta_arg_syntax_error();
	} else if (c == '\n') {
	  /* nop */
	} else {
	  n_meta_args++;
	  state = INSIDE;
	  META_ARG_ADDC(c);
	}
      } else {
	n_meta_args++;
	state = INSIDE;
	META_ARG_ADDC(c);
      }
    } else if (state == INSIDE) {
      if (c == ' ' || c == '\t') {
	state = OUTSIDE;
	META_ARG_ADDC('\0');
      } else if (c == '\n') {
	META_ARG_ADDC('\0');
	break;
      } else if (c == '\'') {
	state = SINGLE_QUOTED;
      } else if (c == '\"') {
	state = DOUBLE_QUOTED;
      } else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) {
	  meta_arg_syntax_error();
	} else if (c == '\n') {
	  /* nop */
	} else {
	  META_ARG_ADDC(c);
	}
      } else {
	META_ARG_ADDC(c);
      }
    } else if (state == SINGLE_QUOTED) {
      if (c == '\'') {
	state = INSIDE;
      } else {
	META_ARG_ADDC(c);
      }
    } else { /* if (state == DOUBLE_QUOTED) */
      if (c == '\"') {
	state = INSIDE;
      } else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) meta_arg_syntax_error();
	META_ARG_ADDC(c);
      } else {
	META_ARG_ADDC(c);
      }
    }
  }
  if (state != INSIDE && state != OUTSIDE) meta_arg_syntax_error();
  if (c == EOF) meta_arg_syntax_error();

  /* Renew argc and argv. */
  new_argv = (char**)xmalloc(sizeof(char*) * (argc + n_meta_args));
  if (new_argv == NULL) meta_arg_error();
  new_argv[0] = argv[0];

  state = OUTSIDE;
  j = 1;
  for (k = 0; k < i; k++) {
    if (state == OUTSIDE) {
      new_argv[j++] = buf + k;
      if (buf[k]) state = INSIDE;
    } else {
      if (!buf[k]) state = OUTSIDE;
    }
  }
  for (j = 2; j <= argc; j++) {
    new_argv[n_meta_args + j - 1] = argv[j];
  }
  *pargc = argc + n_meta_args - 1;
  *pargv = new_argv;
  fclose(fp);
  return n_meta_args;
}

/*=====================================================
 * meta_skip_shebang(fp) - skip the sharp-bang line
 * 
 *====================================================*/
int
meta_skip_shebang(FILE *fp)
{
  int c, ret;
  enum {
    OUTSIDE,
    INSIDE,
    SINGLE_QUOTED,
    DOUBLE_QUOTED,
  } state = OUTSIDE;

  ret = skip_meta_shebang_line(fp);
  if (ret == NOT_SHEBANG) return FALSE;
  else if (ret == NOT_META) return TRUE;
  else if (ret == EOF_REACHED) meta_arg_syntax_error();
 
  /* Parse the second line. */
  /* parser. */
  while ((c = getc(fp)) != EOF) {
    if (state == OUTSIDE) {
      if (c == ' ' || c == '\t') {
	continue;
      } else if (c == '\n') {
	break;
      } else if (c == '\'') {
	state = SINGLE_QUOTED;
      } else if (c == '\"') {
	state = DOUBLE_QUOTED;
      } else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) {
	  meta_arg_syntax_error();
	} else if (c == '\n') {
	  /* nop */
	} else {
	  state = INSIDE;
	}
      } else {
	state = INSIDE;
      }
    } else if (state == INSIDE) {
      if (c == ' ' || c == '\t') {
	state = OUTSIDE;
      } else if (c == '\n') {
	break;
      } else if (c == '\'') {
	state = SINGLE_QUOTED;
      } else if (c == '\"') {
	state = DOUBLE_QUOTED;
      } else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) {
	  meta_arg_syntax_error();
	}
      }
    } else if (state == SINGLE_QUOTED) {
      if (c == '\'') {
	state = INSIDE;
      }
    } else { /* if (state == DOUBLE_QUOTED) */
      if (c == '\"') {
	state = INSIDE;
      } else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) meta_arg_syntax_error();
      }
    }
  }
  if (state != INSIDE && state != OUTSIDE) meta_arg_syntax_error();
  if (c == EOF) meta_arg_syntax_error();
  return TRUE;
}
/*====================================================
 * the main function for debugging.
 *===================================================*/
#ifdef META_ARG_TEST
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
#endif /* def META_ARG_TEST */
/* end of meta_arg.c */
