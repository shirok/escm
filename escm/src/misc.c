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

#ifndef FALSE
#  define FALSE 0
#endif /* FALSE */
#ifndef TRUE
#  define TRUE (!FALSE)
#endif /* TRUE */

int cgi_header_flag = FALSE;

/* cgi_html_header(outp)
 */
void
cgi_html_header(FILE *outp)
{
  fputs("Content-type: text/html\r\n\r\n", outp);
  fflush(outp);
  cgi_header_flag = TRUE;
}

/* cgi_error(fmt, ...) - print a warning message and exit the program.
 */
void cgi_error(const char *fmt, ...)
{
  va_list ap;
  char* iscgi;
  FILE* fp;

  va_start(ap, fmt);
  iscgi = getenv("GATEWAY_INTERFACE");
  fp = iscgi ? stdout : stderr;
  if (iscgi && !cgi_header_flag) {
    fputs("Content-type: text/html\r\n\r\n", stdout);
    fputs("<html><body><p>", stdout);
    cgi_header_flag = TRUE;
  }
  fprintf(fp, "%s: ", cgi_prog);
  if (cgi_file) fprintf(fp, "%s: ", cgi_file);
  vfprintf(fp, fmt, ap);
  if (cgi_header_flag) fputs("</p></body></html>\n", stdout);
  exit(iscgi ? EXIT_SUCCESS : EXIT_FAILURE);
}
/* end of misc.c */
