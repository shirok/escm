/* misc.c - miscellaneous functions
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
#endif /* HAVE_STDLIB_H */

#include <stdarg.h>

#include "escm.h"
#include "misc.h"

static int header = FALSE;

/* escm_html_header(&lang, outp)
 */
void
escm_html_header(const struct escm_lang *lang, FILE *outp)
{
  fputs("Content-type: text/html\r\n\r\n", outp);
  fflush(outp);
  header = TRUE;
}
/* escm_text_header(&lang, outp)
 */
void
escm_text_header(const struct escm_lang *lang, FILE *outp)
{
  fputs("Content-type: text/plain\r\n\r\n", outp);
  fflush(outp);
  header = TRUE;
}

/* escm_error(fmt, ...) - print a warning message and exit the program.
 */
void escm_error(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
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
/* end of misc.c */
