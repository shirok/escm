/* cmdline.c - parser of a command line
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#if defined(HAVE_STDLIB_H)
#  include <stdlib.h>
#else /* not defined(HAVE_STDLIB_H) */
#  define EXIT_SUCCESS 0
#  define EXIT_FAILURE 1
void exit(int status);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
#endif /* HAVE_STDLIB_H */

#if defined(HAVE_STRING_H)
#  include <string.h>
#elif defined(HAVE_STRINGS_H)
#  include <string.sh>
#else
# if defined(HAVE_STRCHR)
char *strchr(const char *s, int c);
# else /* !defined(HAVE_STRCHR) */
#  define strchr(s, c) index((s), (c))
# endif /* defined(HAVE_STRCHR) */
#endif /* defined(HAVE_STRING_H) && defined(HAVE_STRINGS_H) */
#include <ctype.h>

#include "misc.h"
#if !defined(XERROR)
# define XERROR(msg) { perror(msg) ; exit(EXIT_FAILURE) }
#endif /* !defined(XERROR) */

#if !defined(XMALLOC)
# define XMALLOC(type, n) ((type *) xmalloc((n) * (sizeof(type))))
# define XREALLOC(type, p, n) ((type *) xrealloc((p), (n) * (sizeof(type))))
#endif /* !defined(XMALLOC) */

/* xmalloc(size) - wrapper of malloc() 
 */
void *
xmalloc(size_t size)
{
  void *ret;

  ret = malloc(size);
  if (ret == NULL) XERROR("memory exhausted");
  return ret;
}
/* xrealloc(ptr, size) - wrapper of realloc()
 */
void *
xrealloc(void *ptr, size_t size)
{
  void *ret;

  if (ptr == NULL) ret = malloc(size);
  else ret = realloc(ptr, size);
  if (ret == NULL) XERROR("memory exhausted");
  return ret;
}

/* To use the same routine for strings and files,
 * a wrapper function is needed.
 */
static const char* inptr = NULL;
static FILE * infp = NULL;
/* get_char() - get a character from a string or a file. */
static int
get_char(void)
{
  if (inptr) {
    int c;
    c = *inptr;
    inptr++;
    if (c) {
      return c;
    } else {
      return EOF;
    }
  } else {
    return getc(infp);
  }
}
/* init_get_char(fp, ptr) - intialize get_char(). */
static void
init_get_char(FILE* fp, const char* ptr)
{
  if (fp) {
    inptr = NULL;
    infp = fp;
  } else {
    infp = NULL;
    inptr = ptr;
  }
}

static char * outbuf = NULL;
static size_t idx = 0;
static size_t size = 0;
/* add_char(c) - add a character to outbuf[]. */
static void
add_char(int c)
{
  if (idx == size) {
    size += BUFSIZ;
    outbuf = XREALLOC(char, outbuf, size);
  }
  outbuf[idx++] = (char) c;
}
/* init_add_char() - initialize add_char(). */
static void
init_add_char(void)
{
  idx = 0;
  size = 0;
  outbuf = NULL;
}

static int
parse_line(void)
{
  enum {
    OUTSIDE,
    INSIDE,
    SINGLE,
    DOUBLE,
  } state = OUTSIDE;
  int c;
  int argc = 0;

  init_add_char();
  while ((c = get_char()) != EOF && c != '\0') {
    if (state == OUTSIDE) {
      if (c == '\"') {
	state = DOUBLE;
	argc++;
      } else if (c == '\'') {
	state = SINGLE;
	argc++;
      } else if (c == '\\') {
	c = get_char();
	if (c == EOF || c == '\0') break;
	else if (c == '\n') continue;
	state = INSIDE;
	argc++;
	add_char(c);
      } else if (c == '\n') break;
      else if (isspace(c)) continue;
      else {
	state = INSIDE;
	argc++;
	add_char(c);
      }
    } else if (state == INSIDE) {
      if (c == '\"') state = DOUBLE;
      else if (c == '\'') state = SINGLE;
      else if (c == '\\') {
	c = get_char();
	if (c == EOF || c == '\0') break;
	else add_char(c);
      } else if (isspace(c)) {
	add_char('\0');
	state = OUTSIDE;
	if (c == '\n') break;
      } else add_char(c);
    } else if (state == DOUBLE) {
      if (c == '\"') state = INSIDE;
      else if (c == '\\') {
	c = get_char();
	if (c == EOF || c == '\0') break;
	else if (c == '\n') continue;
	else add_char(c);
      } else add_char(c);
    } else { /* state == SINGLE */
      if (c == '\'') state = INSIDE;
      else add_char(c);
    }
  }
  if (state == INSIDE) add_char('\0');
  if (state == DOUBLE || state == SINGLE) {
    XERROR("unterminated string");
  }
  if (c == '\0') {
    XERROR("invalid character");
  }
  return argc;
}

/* parse_cmdline(line) - parse and tokenize a command line LINE. */
char**
parse_cmdline(const char *line)
{
  int argc;
  char** argv;
  char* p;
  int i;

  init_get_char(NULL, line);
  argc = parse_line();
  argv = XMALLOC(char *, argc + 1);
  argv[0] = outbuf;
  p = outbuf;
  for (i = 1; i < argc; i++) {
    p = strchr(p, '\0');
    p++;
    argv[i] = p;
  }
  argv[argc] = NULL;
  return argv;
}

/* parse_shebang(script, &argc, &argv) - tokenize the sharp-bang line.
 * This functions is used to parse first lines of a script file
 * preceded by the meta-switch or those of the file specified by
 * PATH_TRANSLATED.
 */
FILE*
parse_shebang(const char* prog, const char* script, int* pargc, char*** pargv)
{
  FILE* fp = NULL;
  int c;
  int argc = 0;
  char** argv;
  int i;
  char* p;

  fp = fopen(script, "r");
  if (fp == NULL) {
    XERROR("can't open the script file.");
  }
  c = getc(fp);
  if (c != '#') {
    ungetc(c, fp);
  } else {
    c = getc(fp);
    if (c == '!') {
      init_get_char(fp, NULL);
      argc = parse_line();
    } else {
      /* There is no way to push back more than one character,
       * so skip a line. */
      ungetc(c, fp);
      while ((c = getc(fp)) != EOF && c != '\n') {
	/* nop */
      }
    }
  }
  if (argc == 0) {
    argc = 2;
  } else {
    argc += 1;
  }
  *pargc = argc;
  argv = XMALLOC(char*, argc + 1);
  argv[0] = (char*) prog;
  argv[argc - 1] = (char*) script;
  argv[argc] = NULL;
  if (argc > 2) {
    p = outbuf;
    argc -= 1;
    for (i = 1; i < argc; i++) {
      p = strchr(p, '\0');    
      p++;
      argv[i] = p;
    }
  }
  *pargv = argv;
  return fp;
}
/* end of cmdline.c */
