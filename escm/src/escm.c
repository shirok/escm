/************************************************
 * escm.c - preprocess XML with scheme or another
 * interpreter language.
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 ***********************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include "escm.h"
#include "misc.h"

/* Used to make error messages. */
const char *escm_prog = NULL;
const char *escm_file = NULL;
int escm_lineno = 0;
#ifdef ENABLE_CGI
const char *escm_cgi = NULL;
#endif /* ENABLE_CGI */

/* put_string(str, outp) - escape str and put it.
 * This function is not used in this file, but will be useful.
 */
static void
put_string(const char *str, FILE *outp)
{
  const char *p = str;
  fputc('\"', outp);
  while (*p) {
    if (*p == '\"' || *p == '\\') fputc('\\', outp);
    fputc(*p++, outp);
  }
  fputc('\"', outp);
}
/* put_variable(lang, var, outp) - put var as a variable name.
 */
static void
put_variable(const struct escm_lang *lang, const char *var, FILE *outp)
{
  const char *p;
  if (lang->scm_p) fputs(var, outp);
  else {
    int flag = FALSE;
    for (p = var; *p; p++) {
      if (!flag) {
	if (isalpha(*p) || *p == '_') {
	  fputc(tolower(*p), outp);
	  flag = TRUE;
	}
      } else if (isalnum(*p)) {
	fputc(tolower(*p), outp);
      } else if (*p == '-' || *p == '_') {
	fputc('_', outp);
      }
    }
  }
}
/* escm_bind(lang, var, val, outp) - bind var to val in lang
 */
void
escm_bind(const struct escm_lang *lang, const char *var, const char *val, FILE *outp)
{
  if (lang->bind.prefix) fputs(lang->bind.prefix, outp);
  put_variable(lang, var, outp);
  if (lang->bind.infix) fputs(lang->bind.infix, outp);
  if (val == NULL) fputs(lang->nil, outp);
  else put_string(val, outp);
  if (lang->bind.suffix) fputs(lang->bind.suffix, outp);
  fputc('\n', outp);
}
/* escm_assign(lang, var, val, outp) - assign var to val in lang
 */
void
escm_assign(const struct escm_lang *lang, const char *var, const char *val, FILE *outp)
{
  if (lang->assign.prefix) fputs(lang->assign.prefix, outp);
  put_variable(lang, var, outp);
  if (lang->assign.infix) fputs(lang->assign.infix, outp);
  if (val == NULL) fputs(lang->nil, outp);
  else put_string(val, outp);
  if (lang->assign.suffix) fputs(lang->assign.suffix, outp);
  fputc('\n', outp);
}
#ifdef ENABLE_CGI
/* escm_bind_query_string(lang, outp) - bind the query string to QUERY_STRING
 * when the method is POST. */
void
escm_bind_query_string(const struct escm_lang *lang, FILE *outp)
{
  const char *content_length;
  char *p;
  long llen;
  int len;
  int c;

  content_length = getenv("CONTENT_LENGTH");
  if (content_length == NULL)
    escm_error(_("inconsistent environment"));
  else {
    if (lang->bind.prefix) fputs(lang->bind.prefix, outp);
    put_variable(lang, "QUERY_STRING", outp);
    if (lang->bind.infix) fputs(lang->bind.infix, outp);
    llen = strtol(content_length, &p, 10);
    if (*p == '\0') {
      fputc('\"', outp);
      len = (int) llen;
      while ((c = getc(stdin)) != EOF && len-- > 0) {
	fputc(c, outp);
      }
      fputc('\"', outp);
    } else {
      fputs(lang->nil, outp);
    }
    if (lang->bind.suffix) fputs(lang->bind.suffix, outp);
    fputc('\n', outp);
  }
}
#endif /* ENABLE_CGI */
/* escm_init(&lang, outp) - initialize the backend interpreter.
 */
void
escm_init(const struct escm_lang *lang, FILE *outp)
{
  if (lang->init) {
    fputs(lang->init, outp);
    fputc('\n', outp);
  }
}
/* escm_finish(&lang, outp) - finalize the backend interpreter.
 */
void
escm_finish(const struct escm_lang *lang, FILE *outp)
{
  if (lang->finish) {
    fputs(lang->finish, outp);
    fputc('\n', outp);
  }
}
/* escm_preproc(&lang, inp, outp) - the preprocessor.
 */
#define push_char(c) {\
 if (stackptr == 31) goto empty;\
 stack[stackptr++] = c;\
}
void
escm_preproc(const struct escm_lang *lang, FILE *inp, FILE *outp)
{
  int in_string = FALSE;
  enum {
    LITERAL,
    CODE,
    DISPLAY,
    FORMAT,
  } state = LITERAL;
  int c;
  const char *p;
  int str_keep_lineno = 0;
  int tag_keep_lineno = 0;
  char stack[64] = "<?";
  int stackptr = 0, i = 0;

  fputs(lang->literal.prefix, outp);
  while ((c = getc(inp)) != EOF) {
    if (c == '\n') escm_lineno++;
    if (state == LITERAL) {
      if (c == '<') {
	stackptr++;
	c = getc(inp);
	if (c != '?') goto empty;
	stackptr++;
	p = lang->name;
	while ((c = getc(inp)) == *p) {
	  push_char(c);
	  p++;
	}
	if (*p != '\0') goto empty;
	if (isspace(c)) {
	  stackptr = 0;
	  state = CODE;
	  fputs(lang->literal.suffix, outp);
	  fputc('\n', outp);
	  tag_keep_lineno = escm_lineno;
	  if (c == '\n') escm_lineno++;
	  continue;
	}
	if (c != ':') goto empty;
	push_char(':');
	c = getc(inp);
	if (c != 'd') goto empty;
	push_char('d');
	c = getc(inp);
	if (isspace(c)) {
	  if (!lang->display.prefix) goto empty;
	  stackptr = 0;
	  state = DISPLAY;
	  fputs(lang->literal.suffix, outp);
	  fputc('\n', outp);
	  fputs(lang->display.prefix, outp);
	  tag_keep_lineno = escm_lineno;
	  if (c == '\n') escm_lineno++;
	  continue;
	}
	if (c != ':') goto empty;
	push_char(':');
	i = stackptr;
	for (;;) {
	  c = getc(inp);
	  if (c == '\"') goto empty;
	  else if (isspace(c)) break;
	  push_char(c);
	}
	if (!lang->format.infix) goto empty;
	state = FORMAT;
	fputs(lang->literal.suffix, outp);
	fputc('\n', outp);
	fputs(lang->format.prefix, outp);
	tag_keep_lineno = escm_lineno;
	if (c == '\n') escm_lineno++;
	for (/**/; i < stackptr; i++) {
	  fputc(stack[i], outp);
	}
	stackptr = 0;
	fputs(lang->format.infix, outp);
	continue;
      empty:
	for (i = 0; i < stackptr; i++)
	  fputc(stack[i], outp);
	stackptr = 0;
	if (c == EOF) break;
	else ungetc(c, inp);
      } else {
	if (c == '\\' || c == '\"') {
	  fputc('\\', outp);
	  fputc(c, outp);
	} else if (c == '\n') {
	  escm_lineno++;
	  fputc('\\', outp);
	  fputc('n', outp);
	  fputs(lang->literal.suffix, outp);
	  fputc('\n', outp);
	  fputs(lang->literal.prefix, outp);
	} else {
	  fputc(c, outp);	  
	}
      }
    } else {
      if (in_string) {
	if (c == '\\') {
	  fputc('\\', outp);
	  c = getc(inp);
	  if (c == EOF) {
	    fputc('\\', outp);
	    break;
	  }
	} else if (c == '\"') in_string = FALSE;
	fputc(c, outp);
      } else {
	if (c == '?') {
	  c = getc(inp);
	  if (c == EOF) break;
	  else if (c == '>') {
	    if (state == DISPLAY) fputs(lang->display.suffix, outp);
	    else if (state == FORMAT) fputs(lang->format.suffix, outp);
	    fputc('\n', outp);
	    fputs(lang->literal.prefix, outp);
	    state = LITERAL;
	    continue;
	  } else fputc('?', outp);
	} else if (c == '\"') {
	  str_keep_lineno = escm_lineno;
	  in_string = TRUE;
	}
	fputc(c, outp);
      }
    }
  }
  if (state == LITERAL) fputs(lang->literal.suffix, outp);
  else {
    if (in_string) {
      fputc('\"', outp);
      escm_lineno = str_keep_lineno;
      escm_error(_("unterminated string"));
    }
    if (state == DISPLAY) fputs(lang->display.suffix, outp);
    escm_lineno = tag_keep_lineno;
    escm_error(_("unterminated instruction"));
  }
  fputc('\n', outp);
}
/* end of escm.c */
