/************************************************
 * escmcgi.c - functions for the CGI tool.
 * $Id$
 * Copyright (c) 2003-2004 TAGA Yoshitaka <tagga@tsuda.ac.jp>
 ***********************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include "escm.h"
#include "escmcgi.h"

#define setter bind
/* escm_header(lang, outp)
 */
void
escm_header(const struct escm_lang *lang, FILE *outp)
{
  fputs(lang->literal.prefix, outp);
  if (lang->newline) {
    fputs("Content-type: text/html", outp);
    fputs(lang->literal.suffix, outp);
    fputs(lang->newline, outp);
    fputs(lang->newline, outp);
    fputc('\n', outp);
  } else {
    fputs("Content-type: text/html\r\n\r\n", outp);
    fputs(lang->literal.suffix, outp);
    fputc('\n', outp);
  }
}
/* escm_query_string(lang, outp) - bind the query string to QUERY_STRING
 * when the method is POST. */
int
escm_query_string(const struct escm_lang *lang, FILE *outp)
{
  const char *content_length;
  const char *method;
  char *p;
  long llen;
  int len;
  int c;

  method = getenv("REQUEST_METHOD");
  if (method && method[0] == 'P') {
    content_length = getenv("CONTENT_LENGTH");
    if (content_length == NULL) {
      return FALSE;
    } else {
      if (lang->setter.prefix) fputs(lang->setter.prefix, outp);
      fputs("escm_query_string", outp);
      if (lang->setter.infix) fputs(lang->setter.infix, outp);
      llen = strtol(content_length, &p, 10);
      if (*p == '\0') {
	fputc('"', outp);
	len = (int) llen;
	while ((c = getc(stdin)) != EOF && len-- > 0) {
	  /* better replace the next line with an error check. */
	  if (c == '"' || c == '\\') fputc('\\', outp);
	  fputc(c, outp);
	}
	fputc('"', outp);
      } else {
	fputs(lang->nil, outp);
      }
      if (lang->setter.suffix) fputs(lang->setter.suffix, outp);
      fputc('\n', outp);
    }
  } else {
    p = getenv("QUERY_STRING");
    if (lang->setter.prefix) fputs(lang->setter.prefix, outp);
    fputs("escm_query_string", outp);
    if (lang->setter.infix) fputs(lang->setter.infix, outp);
    if (p == NULL) fputs(lang->nil, outp);
    else escm_puts(p, outp);
    if (lang->setter.suffix) fputs(lang->setter.suffix, outp);
    fputc('\n', outp);
  }
  return TRUE;
}
#undef setter

/* escm_skip_shebang(inp) - skip the sharp-bang line.
 */
void
escm_skip_shebang(FILE *inp)
{
  int c;
  c = fgetc(inp);
  if (c != '#') {
    ungetc(c, inp);
    return;
  }
  while ((c = fgetc(inp)) != EOF && c != '\n')
    ;
}
/* escm_add_header(lang, inp, outp) - add the content header if necessary
 */
void
escm_add_header(const struct escm_lang *lang, FILE *inp, FILE *outp)
{
  int c;
  c = fgetc(inp);
  ungetc(c, inp);
  if (c == '<') escm_header(lang, outp);
}

/* end of escmcgi.c */
