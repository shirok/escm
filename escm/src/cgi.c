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

enum cgi_header_type {
  CGI_HEADER_NONE,
  CGI_HEADER_HTML,
};

static int in_cgi = FALSE;
static int cached = FALSE;
static enum cgi_header_type header_type = CGI_HEADER_NONE;

/* escm_is_cgi() - return TRUE if invoked in a CGI script, FALSE otherwise.
 */
int
escm_is_cgi(void)
{
  if (cached) return in_cgi;
  cached = TRUE;
  if (getenv("GATEWAY_INTERFACE")) in_cgi = TRUE;
  return in_cgi;
}

/* escm_header(&lang, outp)
 */
void
escm_header(const struct escm_lang *lang, FILE *outp)
{
  fputs(lang->literal_prefix, outp);
  fputs("Content-type: text/html\\r\\n\\r\\n", outp);
  fputs(lang->literal_suffix, outp);
  fputc('\n', outp);
  header_type = CGI_HEADER_HTML;
}
/* escm_error(fmt, ...) - print a warning message and exit the program.
 * The message is specified by strerror(errno) if msg is NULL.
 */
void escm_error(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  if (fmt == NULL) fmt = strerror(errno);
  if (escm_is_cgi()) {
    if (header_type != CGI_HEADER_HTML) {
      fputs("Content-type: text/html\r\n\r\n", stdout);
      fputs("<html><body>", stdout);
    }
    fputs("<p>", stdout);
    vfprintf(stdout, fmt, ap);
    fputs("</p></body></html>\n", stdout);
    exit(EXIT_SUCCESS);
  } else {
    vfprintf(stderr, fmt, ap);
    exit(EXIT_FAILURE);
  }
}
/* end of cgi.c */
