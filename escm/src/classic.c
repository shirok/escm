/* classic.c - convert the original escm format into the XML format.
 * This file was derived from escm.c of escm 1.1.
 */
/*
 * eScheme - Embedded Scheme code processor.
 *
 *  Copyright (c) 2003 TAGA Yoshitaka <tagga@tsuda.ac.jp>
 *  Copyright (c) 2000-2001 Shiro Kawai  shiro@acm.org
 *
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify,
 *  merge, publish, distribute, sublicense, and/or sell copies of
 *  the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 *  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 *  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 *  OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 *  $Id$
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#else
void exit(int status);
#  define EXIT_SUCCESS 0
#  define EXIT_FAILURE 1
#endif /* HAVE_STDLIB_H */
#if defined(HAVE_UNISTD_H)
#  include <unistd.h>
#else
#  include <getopt.h>
#endif /* defined(HAVE_UNISTD_H) */


#if !defined(ESCM_PATH)
#  define ESCM_PATH "/usr/local/bin/aescm"
#endif /* !defined(ESCM_PATH) */

void preprocess(FILE *, FILE *, int);

void
unexpected_eof(char **argv, int lineno)
{
  fprintf(stderr, "%s: %s: %d: unexpected eof\n",
	  argv[0], argv[1] ? argv[1] : "stdin", lineno);
  exit(EXIT_FAILURE);
}
void
usage(char **argv)
{
  fprintf(stderr,
	  "Usage: %s [-p PATH] [-o OUTFILE] [INFILE]\n"
	  "Convert the original escm format into the XML format.\n",
	  argv[0]);
  exit(EXIT_FAILURE);
}
void
cant_open(char ** argv, char *file)
{
  fprintf(stderr, "%s: can't open - %s\n", argv[0], file);
  exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
  int c;
  int has_opt = 0;
  FILE *infp = stdin;
  FILE *outfp = stdout;
  char *path = ESCM_PATH;

  while ((c = getopt(argc, argv, "o:p:")) != -1) {
    switch (c) {
    case 'o':
      outfp = fopen(optarg, "w");
      if (!outfp) cant_open(argv, optarg);
      break;
    case 'p':
      path = optarg;
      break;
    default:
      usage(argv);
    }
  }

  if (optind < argc) {
    infp = fopen(argv[optind], "r");
    if (!infp) cant_open(argv, argv[optind]);
  }

  /* the first line */
  c = getc(infp);
  if (c == EOF) unexpected_eof(argv, 1);
  else if (c != '#') goto skip;
  fputc('#', outfp);

  c = getc(infp);
  if (c == EOF) unexpected_eof(argv, 1);
  else if (c != '!') goto skip;
  fputc('!', outfp);

  /* skip blanks if any */
  for (;;) {
    c = getc(infp);
    if (c == EOF) unexpected_eof(argv, 1);
    else if (c != ' ' && c != '\t') break;
  }
  fputs(ESCM_PATH, outfp);

  /* skip the old path */
  for (;;) {
    c = getc(infp);
    if (c == EOF) unexpected_eof(argv, 1);
    else if (c == ' ' || c == '\t') break;
    else if (c == '\n') goto second_line;
  }
  for (;;) {
    c = getc(infp);
    if (c == EOF) unexpected_eof(argv, 1);
    else if (c != ' ' && c != '\t') break;
    else if (c == '\n') goto second_line;
  }
  /* use the meta-argument functionality */
  fputs(" \\\n ", outfp);
  has_opt = 1;

  /* copy chars before newline */
  fputc(c, outfp);
  for (;;) {
    c = getc(infp);
    if (c == EOF) unexpected_eof(argv, 1);
    else if (c == '\n') break;
    else fputc(c, outfp);
  }

 second_line:
  c = getc(infp);
  if (c != '#') {
    fputc('\n', outfp);
  } else {
    c = getc(infp);
    if (c != '?') {
      fputs("\n#", outfp);
    } else {
      if (!has_opt) fputs(" \\\n", outfp);
      else fputc(' ', outfp);
      fputs(" -i \"", outfp);
      for (;;) {
	c = getc(infp);
	if (c == EOF) unexpected_eof(argv, 2);
	else if (c == '\n') {
	  fputc('\"', outfp);
	  break;
	} else if (c == '\\' || c == '\"') {
	  fputc('\\', outfp);
	}
	fputc(c, outfp);
      }
    }
  }

 skip:
  preprocess(infp, outfp, c);
  return 0;
}

void
preprocess(FILE * in, FILE * out, int nextchar)
{
  enum {
    IN_LITERAL,
    IN_SCHEME,
    IN_EXPR
  } status = IN_LITERAL;
  int c = nextchar, c1;

  do {
    if (status == IN_LITERAL) {
      switch (c) {
      case '<':
	c1 = getc(in);
	if (c1 == '?') {
	  int c2 = getc(in);
	  if (c2 == '=') {
	    fputs("<?scm:d ", out);
	    status = IN_EXPR;
	    continue;
	  } else if (c2 == EOF) {
	    fputs("<?", out); /* unterminated <? */
	    break;
	  } else {
	    /* ignore a character for compatibility */
	    fputs("<?scm (begin ", out);
	    status = IN_SCHEME;
	    continue;
	  }
	} else if (c1 == EOF) {
	  fputc(c, out);
	  break;
	} else {
	  fputc(c, out);
	  fputc(c1, out);
	  continue;
	}
      default:
	fputc(c, out);
	continue;
      }
    } else {
      if (c == '!') {
	c1 = getc(in);
	if (c1 == '>') {
	  /* End of Scheme part. */
	  if (status == IN_EXPR) fputs("?>", out);
	  else fputs(")?>", out);
	  status = IN_LITERAL;
	  continue;
	} else if (c == EOF) {
	  fputc(c, out);
	  break;
	} else {
	  fputc(c, out);
	  fputc(c1, out);
	  continue;
	}
      } else {
	fputc(c, out);
	continue;
      }
    }
  }
  while ((c = getc(in)) != EOF);
}
/* end of classic.c */
