/****************************************************
 * cgi.c - CGI utilities
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 ****************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STRING_H
# include <string.h>
#endif /* HAVE_STRING_H */

#include <stdarg.h>
#include <errno.h>

#include "escm.h"

static int header = FALSE;

/* escm_header(&lang, outp)
 */
void
escm_header(const struct escm_lang *lang, FILE *outp)
{
  fputs("Content-type: text/html\r\n\r\n", outp);
  fflush(outp);
  header = TRUE;
}
/* escm_error(fmt, ...) - print a warning message and exit the program.
 * The message is specified by strerror(errno) if msg is NULL.
 */
void escm_error(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  if (fmt == NULL) fmt = strerror(errno);
  if (escm_cgi) {
    if (!header) {
      fputs("Content-type: text/html\r\n\r\n", stdout);
      fputs("<html><body>", stdout);
    }
    printf("<p>%s: ", escm_prog);
    if (escm_file) printf("%s: ", escm_file);
    if (escm_lineno) printf("%d: ", escm_lineno);
    vfprintf(stdout, fmt, ap);
    fputs("</p></body></html>\n", stdout);
    exit(EXIT_SUCCESS);
  } else {
    fprintf(stderr, "%s: ", escm_prog);
    if (escm_file) fprintf(stderr, "%s: ", escm_file);
    if (escm_lineno) fprintf(stderr, "%d: ", escm_lineno);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
  }
}
/* end of cgi.c */
