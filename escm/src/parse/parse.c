/* parse.c - parse a string as command line
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include "parse.h"

#ifndef xmalloc
#define xmalloc(size) malloc(size)
#endif /* xmalloc */
#ifndef xerror
#define xerror(str) perror(str)
#endif /* xerror */

/* unquote_tokenize(dst, src) - copy strings from SRC to DST,
 * unquotefy all, and tokenize them.
 * Return the number of tokens.
 */
static int
unquote_tokenize(char *dst, char *src)
{
  enum {
    BLANK,
    SINGLE,
    DOUBLE,
    OTHER,
    NONE,
  } state = BLANK;
  int cnt = 0;

  while (*src) {
    if (state == BLANK) {
      if (isspace(*src)) {
	/* nop */
      } else if (*src == '\'') {
	state = SINGLE;
	cnt++;
      } else if (*src == '\"') {
	state = DOUBLE;
	cnt++;
      } else if (*src == '\\') {
	src++;
	if (*src == '\0') break;
	state = OTHER;
	cnt++;
	*dst++ = *src;
      } else {
	state = OTHER;
	cnt++;
	*dst++ = *src;
      }
    } else if (state == SINGLE) {
      if (isspace(*src)) {
	*dst++ = *src;
      } else if (*src == '\'') {
	state = NONE;
      } else if (*src == '\"') {
	*dst++ = *src;
      } else if (*src == '\\') {
	*dst++ = *src;
      } else {
	*dst++ = *src;
      }
    } else if (state == DOUBLE) {
      if (isspace(*src)) {
	*dst++ = *src;
      } else if (*src == '\'') {
	*dst++ = *src;
      } else if (*src == '\"') {
	state = NONE;
      } else if (*src == '\\') {
	src++;
	if (*src == '\0') break;
	*dst++ = *src;
      } else {
	*dst++ = *src;
      }
    } else if (state == OTHER) {
      if (isspace(*src)) {
	*dst++ = '\0';
	state = BLANK;
      } else if (*src == '\'') {
	state = SINGLE;
      } else if (*src == '\"') {
	state = DOUBLE;
      } else if (*src == '\\') {
	src++;
	if (*src == '\0') break;
	*dst++ = *src;
      } else {
	*dst++ = *src;
      }
    } else { /* NONE */
      if (isspace(*src)) {
	*dst++ = '\0';
	state = BLANK;
      } else if (*src == '\'') {
	state = SINGLE;
      } else if (*src == '\"') {
	state = DOUBLE;
      } else if (*src == '\\') {
	src++;
	if (*src == '\0') break;
	state = OTHER;
	*dst++ = *src;
      } else {
	state = OTHER;
	*dst++ = *src;
      }
    }
    src++;
  }
  if (state != BLANK) *dst = '\0';
  return cnt;
}

/* next_token(from, to) - return the pointer to the next token
 * in the range of from FROM (exclusively) to TO (exclusively).
 */
static char *
next_token(char *from, char *to)
{
  while (*from != '\0' && from < to)
    from++;
  from++;
  return from >= to ? NULL : from;
}

/* argv = command_line(line, &argc) - parse LINE as a command line,
 * set ARGC to the number of arguments, and return the pointer to an array
 * of the pointers to arguments.
 */
char **
command_line(char *line, int *argc)
{
  size_t len;
  char *copy;
  char **argv;
  int i;

  len = strlen(line) + 1; /* 1 for '\0' */
  copy = (char *) xmalloc(len);
  if (copy == NULL) xerror("command_line");
  *argc = unquote_tokenize(copy, line);
  argv = (char **) xmalloc(sizeof(char *) * (*argc + 1)); /* 1 for NULL */
  if (argv == NULL) xerror("command_line");
  argv[0] = copy;
  for (i = 1; i < *argc; i++) {
    argv[i] = next_token(argv[i - 1], copy + len);
  }
  argv[i] = NULL;
  return argv;
}

/* argv = shebang(argv, &argc) - parse argv[1] which will be the
 * optional argument of a #! line and remake argc and argv[] so that
 * we can separately specify options.
 */
char **
shebang(char **argv, int *argc)
{
  size_t len;
  int cnt;
  char *copy;
  char **new_argv;
  int i;

  if (*argc <= 2) return argv;

  len = strlen(argv[1]) + 1; /* 1 for '\0' */
  copy = (char *) xmalloc(len);
  if (copy == NULL) xerror("shebang");
  cnt = unquote_tokenize(copy, argv[1]);
  new_argv = (char **) xmalloc(sizeof(char *) * (*argc + cnt));
  if (new_argv == NULL) xerror("shebang");
  new_argv[0] = argv[0];
  new_argv[1] = copy;
  *argc = *argc - 1 + cnt;
  cnt += 1;
  for (i = 2; i < cnt; i++) {
    new_argv[i] = next_token(new_argv[i - 1], copy + len);
  }
  cnt -= 2;
  for (/**/; i < *argc; i++) {
    new_argv[i] = argv[i - cnt];
  }
  new_argv[i] = NULL;
  return new_argv;
}

#ifdef PARSE_MAIN
int main(int argc, char ** argv)
{
  int i;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s \"command line\"\n", argv[0]);
    return 1;
  }
  argv = command_line(argv[1], &argc);
  printf("    int argc = %d;\n", argc);
  printf("    char *const argv[] = {\n");
  for (i = 0; i < argc; i++) {
    printf("        \"%s\",\n", argv[i]);
  }
  printf("        NULL,\n");
  printf("    };\n");
  return 0;
}
#endif /* PARSE_MAIN */

#ifdef PARSE_SHEBANG
int main(int argc, char ** argv)
{
  int i;

  argv = shebang(argv, &argc);
  printf("    int argc = %d;\n", argc);
  printf("    char *const argv[] = {\n");
  for (i = 0; i < argc; i++) {
    printf("        \"%s\",\n", argv[i]);
  }
  printf("        NULL,\n");
  printf("    };\n");
  return 0;
}
#endif /* PARSE_SHEBANG */
/* end of parse.c */
