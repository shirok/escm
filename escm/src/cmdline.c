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
#endif /* defined(HAVE_STRING_H) && defined(HAVE_STRINGS_H) */

#include "misc.h"
#if !defined(XERROR)
# define XERROR perror
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

char **
tokenize_cmd(const char *cmd)
{
  char *p;
  char *str;
  int i, n = 0;
  char **argv = NULL;

  i = 1 + strlen(cmd);
  str = XMALLOC(char, i);
  strcpy(str, cmd);
  p = strtok(str, " \t");
  for (i = 0; /**/; i++, p = strtok(NULL, " \t")) {
    if (i == n) {
      n += 4;
      argv = XREALLOC(char *, argv, n);
    }
    argv[i] = p;
    if (p == NULL) break;
  }
  return argv;
}
/* end of cmdline.c */
