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
char* getenv(const char* name);
#endif /* HAVE_STDLIB_H */
#include <stdarg.h>

#include "misc.h"

static enum {
  NONE,
  TEXT,
  HTML,
} header = NONE;

/* cgi_html_header(outp)
 */
void
cgi_html_header(FILE *outp)
{
  fputs("Content-type: text/html\r\n\r\n", outp);
  fflush(outp);
  header = HTML;
}
/* cgi_text_header(outp)
 */
void
cgi_text_header(FILE *outp)
{
  fputs("Content-type: text/plain\r\n\r\n", outp);
  fflush(outp);
  header = TEXT;
}

/* cgi_error(fmt, ...) - print a warning message and exit the program.
 */
void cgi_error(const char *fmt, ...)
{
  va_list ap;
  char* iscgi;
  FILE* fp;

  iscgi = getenv("GATEWAY_INTERFACE");
  fp = iscgi ? stdout : stderr;
  if (header == HTML) {
    fputs("Content-type: text/html\r\n\r\n", stdout);
    fputs("<html><body><p>", stdout);
  }
  fprintf(fp, "%s: ", cgi_prog);
  if (cgi_file) fprintf(fp, "%s: ", cgi_file);
  if (cgi_lineno) fprintf(fp, "%d: ", cgi_lineno);
  vfprintf(fp, fmt, ap);
  if (header == HTML) fputs("</p></body></html>\n", stdout);
  exit(iscgi ? EXIT_SUCCESS : EXIT_FAILURE);
}
/* end of misc.c */
