/* cgi.c - miscellaneous functions for CGI
 * $Id$
 *
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 * See `COPYING' on the license.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include "cgi.h"

/* cgi_error(fmt, ...) - print an error message and exit. */
void
cgi_error(char *fmt, ...)
{
  FILE *fp;
  int cgi_p;
  va_list args;

  va_start(args, fmt);
  cgi_p = is_cgi();
  fp = cgi_p ? stdout : stderr;
  if (cgi_p) fputs("Content-type: text/plain\r\n\r\n", fp);
  vfprintf(fp, fmt, args);
  va_end(args);
  if (errno) fprintf(fp, ": %s\n", strerror(errno));
  else fputc('\n', fp);
  exit(cgi_p ? 0 : 1);
}

/* is_cgi() - return a true if in a CGI script */
int
is_cgi(void)
{
  return getenv("GATEWAY_INTERFACE") ? 1 : 0;
}
/* end of cgi.c */
