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
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include "escm.h"

enum cgi_header_type {
  CGI_HEADER_NONE,
  CGI_HEADER_PLAIN,
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

/* escm_stderr2stdout() - redirect stderr to stdout.
 */
int
escm_stderr2stdout(void)
{
  return dup2(fileno(stdout), fileno(stderr)) < 0 ? FALSE : TRUE;
}
/* escm_html_header() - send an HTML content-type header.
 */
void
escm_html_header(void)
{
  if (header_type == CGI_HEADER_NONE) {
    header_type = CGI_HEADER_HTML;
    printf("Content-type: text/html\r\n\r\n");
    fflush(stdout);
  }
}
/* escm_plain_header() - send a plain text content-type header.
 */
void
escm_plain_header(void)
{
  if (header_type == CGI_HEADER_NONE) {
    header_type = CGI_HEADER_PLAIN;
    printf("Content-type: text/plain\r\n\r\n");
    fflush(stdout);
  }
}

/* plain_warning(fp, prog, tag, msg) - warning in plain text format.
 */
static void
plain_warning(FILE *fp, const char *prog, const char *tag, const char *msg)
{
  fflush(fp);
  fputs(prog, fp);
  fputs(" - ", fp);
  fputs(tag, fp);
  fputs(": ", fp);
  fputs(msg, fp);
  fputc('\n', fp);
}
/* html_warning(fp, prog, tag, msg) - warning in HTML format.
 */
static void
html_warning(FILE *fp, const char *prog, const char *tag, const char *msg)
{
  fflush(fp);
  fputs("<p><em>", fp);
  fputs(prog, fp);
  fputs("</em> - <strong>", fp);
  fputs(tag, fp);
  fputs(":</strong> ", fp);
  fputs(msg, fp);
  fputs("</p>", fp);
  fputc('\n', fp);
}
/* escm_warning(prog, msg) - print a warning message.
 * The message is specified by strerror(errno) if msg is NULL.
 */
void
escm_warning(const char *prog, const char *msg)
{
  if (msg == NULL) msg = strerror(errno);
  if (escm_is_cgi()) {
    if (header_type == CGI_HEADER_HTML) {
      html_warning(stdout, prog, "warning", msg);
    } else {
      if (header_type == CGI_HEADER_NONE) escm_plain_header();
      plain_warning(stdout, prog, "warning", msg);
    }
  } else {
    plain_warning(stderr, prog, "warning", msg);
  }
}

/* escm_error(prog, msg) - print a warning message and exit the program.
 * The message is specified by strerror(errno) if msg is NULL.
 */
void escm_error(const char *prog, const char *msg)
{
  if (msg == NULL) msg = strerror(errno);
  if (escm_is_cgi()) {
    if (header_type == CGI_HEADER_HTML) {
      html_warning(stdout, prog, "error", msg);
      printf("</body></html>\n");
      exit(EXIT_SUCCESS);
    } else {
      if (header_type == CGI_HEADER_NONE) escm_plain_header();
      plain_warning(stdout, prog, "error", msg);
      exit(EXIT_SUCCESS);
    }
  } else {
    plain_warning(stderr, prog, "error", msg);
    exit(EXIT_FAILURE);
  }
}
/* end of cgi.c */
