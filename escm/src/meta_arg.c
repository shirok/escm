/* meta_argc.c - add meta-argument functionality
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 *
 * fp = meta_expand(&argc, &argv, &scr_argc, &src_argv, optstr);
 */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>

#if defined(HAVE_STDLIB_H)
#  include <stdlib.h>
#else /* !defined(HAVE_STDLIB_H) */
void exit(int status);
#  define EXIT_SUCCESS 0
#  define EXIT_FAILURE 1
#  if !defined(XMALLOC)
void *malloc(size_t size);
void *realloc(void *p, size_t size);
#  endif /* !defined(XMALLOC) */
#endif /* defined(HAVE_STDLIB_H) */

#if !defined(HAVE_STRCHR)
#  define strchr index
#endif /* !defined(HAVE_STRCHR) */
#if defined(HAVE_STRING_H)
#  include <string.h>
#elif defined(HAVE_STRINGS_H)
#  include <strings.h>
#else /* !defined(HAVE_STRING_H) && !defined(HAVE_STRINGS_H) */
char *strchr(const char *s, int c);
#endif /* defined(HAVE_STRING_H) && defined(HAVE_STRINGS_H) */

#include "meta_arg.h"

#ifndef XMALLOC
#  define XMALLOC(type, n) ((type *)malloc((n) * sizeof(type)))
#endif /* !XMALLOC */
#ifndef XREALLOC
#  define XREALLOC(type, p, n) ((type *)realloc(p, (n) * sizeof(type)))
#endif /* !XREALLOC */

#ifndef XPROG
static char *xprog;
#  define XPROG xprog
#endif /* !XPROG */
#ifndef XFILE
static const char *xfile = NULL;
#  define XFILE xfile
#endif /* !XFILE */
#ifndef XLINENO
static int xlineno = 0;
#  define XLINENO xlineno
#endif /* !XLINENO */

#ifndef XERROR1
#define XERROR1(fmt, arg) {\
  fprintf(stderr, "%s: ", XPROG); \
  fprintf(stderr, fmt, arg);\
  fputc('\n', stderr);\
  exit(EXIT_FAILURE);\
}
#endif /* XERROR1 */
#ifndef XERROR0
#define XERROR0(fmt) {\
  fprintf(stderr, "%s: %s %d: ", XPROG, XFILE, XLINENO); \
  fputs(fmt, stderr);\
  fputc('\n', stderr);\
  exit(EXIT_FAILURE);\
}
#endif /* XERROR0 */

/* fp = expand_args(fp) */
#define ADD_CHAR(c) {\
  if (size == i) {\
     size += BUFSIZ;\
     buf = XREALLOC(char, buf, size);\
  }\
  buf[i++] = (char) (c);\
}

/* fp = expand_args(script, &argc, &argv) */
static FILE *
expand_args(char *script, int *pargc, char ***pargv)
{
  enum {
    OUTSIDE,
    INSIDE,
    SINGLE,
    DOUBLE,
  } state = OUTSIDE;
  int c, n = 0, i = 0;
  size_t size = BUFSIZ;
  int keep_lineno = 0;
  char *buf, *p, **argv;
  FILE *fp;

  fp = fopen(script, "r");
  if (fp == NULL) XERROR1(_("can't open %s"), script);
  c = getc(fp);
  if (c != '#') {
    if (c != EOF) ungetc(c, fp);
    fclose(fp);
    return NULL;
  }
  XFILE = script;
  XLINENO = 1;
  c = getc(fp);
  if (c != '!') XERROR0(_("syntax error"));

  buf = XMALLOC(char, BUFSIZ);
  while ((c = getc(fp)) != EOF) {
    if (state == OUTSIDE) {
      if (c == '\"') {
	keep_lineno = XLINENO;
	state = DOUBLE;
	n++;
      } else if (c == '\'') {
	keep_lineno = XLINENO;
	state = SINGLE;
	n++;
      } else if (c == '\\') {
	c = getc(fp);
	if (c == EOF) {
	  XERROR0(_("unexpected eof"));
	} else if (c == '\n') {
	  XLINENO++;
	  continue;
	}
	state = INSIDE;
	n++;
	ADD_CHAR(c);
      } else if (c == '\n') {
	XLINENO++;
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
	if (c == EOF) {
	  XERROR0(_("unexpected eof"));
	} else if (c == '\n') {
	  XLINENO++;
	  continue;
	} else ADD_CHAR(c);
      } else if (c == '\n') {
	XLINENO++;
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
	if (c == EOF) {
	  XERROR0(_("unexpected eof"));
	} else if (c == '\n') {
	  XLINENO++;
	  continue;
	}
	ADD_CHAR(c);
      } else {
	if (c == '\n') XLINENO++;
	ADD_CHAR(c);
      }
    } else { /* state == SINGLE */
      if (c == '\'') state = INSIDE;
      else {
	if (c == '\n') XLINENO++;
	ADD_CHAR(c);
      }
    }
  }
  if (state == DOUBLE || state == SINGLE) {
    XLINENO = keep_lineno;
    XERROR0(_("unterminated string"));
  } else if (state == INSIDE) XERROR0(_("unexpected eof"));

  *pargc = n;
  argv = XMALLOC(char *, n + 1);
  *pargv = argv;
  p = buf;
  for (i = 0; i < n; i++) {
    argv[i] = p;
    while (*p)
      p++;
    p++;
  }
  argv[i] = NULL;
  return fp;
}

/* fp = skip_shebang(script); */
static FILE *
skip_shebang(const char *script)
{
  FILE *fp;
  int c;

  fp = fopen(script, "r");
  if (fp == NULL) XERROR1(_("can't open %s"), script);
  c = getc(fp);
  if (c != '#') {
    if (c != EOF) ungetc(c, fp);
    fclose(fp);
    return NULL;
  }
  while ((c = getc(fp)) != '\n' && c != EOF)
    ;
  return fp;
}

/* fp = meta_expand(&interpc, &interpv, &scriptc, &scriptv, optstr); */
FILE *
meta_expand(int *pinterpc, char ***pinterpv,
	    int *pscriptc, char ***pscriptv,
#ifdef META_ACTION
	    char *optstr, char *path
#else /* !META_ACTION */
	    char *optstr
#endif /* META_ACTION */
	    )
{
  char **argv;
  int argc, i, which = 1;
  FILE *fp = NULL;

  argc = *pinterpc;
  argv = *pinterpv;
  *pscriptc = 0;
  *pscriptv = NULL;

  XPROG = argv[0];
  if (argc <= 1) { /* no shebang script file. */
#ifndef META_ACTION
    return NULL;
#else /* META_ACTION */
    if (!path) return NULL;
    fp = expand_args(path, pinterpc, pinterpv);
    if (!fp) {
      *pinterpc = 2;
      *pinterpv = XMALLOC(char *, 3);
      (*pinterpv)[0] = argv[0];
      (*pinterpv)[1] = path;
      (*pinterpv)[2] = NULL;
    } else if (*pinterpv != argv) {
      *pscriptc = 1;
      *pscriptv = XMALLOC(char *, 2);
      (*pscriptv)[0] = path;
      (*pscriptv)[1] = NULL;
    }
    return fp;
#endif /* !META_ACTION */
  } else if (argc == 2) {
    if (argv[1][0] == '-') { /* no shebang script file. */
      return NULL;
    } else {
      goto script_without_meta_args;
    }
  } else {
    if (argv[1][0] == '\\' && argv[1][1] == '\0') { /* meta-arguments */
      fp = expand_args(argv[2], pinterpc, pinterpv);
      if (*pinterpv != argv) {
	*pscriptc = argc - 2;
	*pscriptv = argv + 2;
      }
      return fp;
    } else if (argv[1][0] != '-') {
      goto script_without_meta_args;
    } else if (argv[2][0] == '-') { /* no shebang script file. */
      return NULL;
    } else {
      /* check if argv[2] is an option argument. */
      char *p, *q;
      for (p = argv[1] + 1; *p; p++) {
	q = strchr(optstr, *p);
	if (!q) {
	  return NULL; /* getopt() will cause an error. */
	} else if (q[1] == ':') {
	  if (p[1] == '\0') return NULL;
	  break;
	}
      }
      which = 2;
    }
  }
 script_without_meta_args:
  fp = skip_shebang(argv[which]);
  if (!fp) return NULL;
  *pinterpc = which;
  *pinterpv = XMALLOC(char *, which + 1);
  for (i = 0; i < which; i++) {
    (*pinterpv)[i] = argv[i];
  }
  (*pinterpv)[i] = NULL;
  *pscriptc = argc - which;
  *pscriptv = argv + which;
  return fp;
}
#ifdef META_DEBUG
int main(int argc, char *argv[])
{
  int s_argc;
  char **s_argv;
  int i;
  FILE *fp;

  fp = meta_interp(&argc, &argv, &s_argc, &s_argv, "ab:");
  printf("interp\n");
  for (i = 0; i < argc; i++) {
    printf("%d => %s\n", i, argv[i]);
  }
  printf("script\n");
  for (i = 0; i < s_argc; i++) {
    printf("%d => %s\n", i, s_argv[i]);
  }
  return 0;
}
#endif /* META_DEBUG */
/* end of meta_arg.c */
