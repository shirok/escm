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

#define SizeOfArray(arr) (sizeof(arr) / (sizeof(arr[0])))

/* Used to make error messages. */
const char *escm_prog = NULL;
const char *escm_file = NULL;
int escm_lineno = 0;

static char * env_to_bind[] = {
  "GATEWAY_INTERFACE",
  "HTTP_ACCEPT_LANGUAGE",
  "HTTP_ACCEPT_CHARSET",
  "HTTP_COOKIE",
  "HTTP_HOST",
  "HTTP_REFERER",
  "HTTP_USER_AGENT",
  /* "QUERY_STRING", */
  "REMOTE_ADDR",
  "REQUEST_METHOD",
};

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
  for (p = var; *p; p++) {
    if (isalnum(*p) || lang->use_hyphen || *p == '_') {
      fputc(*p, outp);
    } else if (*p == '-') {
      fputc('_', outp);
    }
  }
}
/* escm_literal(lang, str, outp) - put a string in lang
 */
void
escm_literal(const struct escm_lang *lang, const char *str, FILE *outp)
{
  if (lang->literal_prefix) fputs(lang->literal_prefix, outp);
  fputs(str, outp);
  if (lang->literal_suffix) fputs(lang->literal_suffix, outp);
  fputc('\n', outp);
}
/* escm_bind(lang, var, val, outp) - bind var to val in lang
 */
void
escm_bind(const struct escm_lang *lang, const char *var, const char *val, FILE *outp)
{
  if (lang->bind_prefix) fputs(lang->bind_prefix, outp);
  put_variable(lang, var, outp);
  if (lang->bind_infix) fputs(lang->bind_infix, outp);
  if (val == NULL) fputs(lang->nil, outp);
  else put_string(val, outp);
  if (lang->bind_suffix) fputs(lang->bind_suffix, outp);
  fputc('\n', outp);
}
/* escm_assign(lang, var, val, outp) - assign var to val in lang
 */
void
escm_assign(const struct escm_lang *lang, const char *var, const char *val, FILE *outp)
{
  if (lang->assign_prefix) fputs(lang->assign_prefix, outp);
  put_variable(lang, var, outp);
  if (lang->assign_infix) fputs(lang->assign_infix, outp);
  if (val == NULL) fputs(lang->nil, outp);
  else put_string(val, outp);
  if (lang->assign_suffix) fputs(lang->assign_suffix, outp);
  fputc('\n', outp);
}
/* escm_init(&lang, outp) - initialize the backend interpreter.
 */
static void
cgi_post(const struct escm_lang *lang, FILE *outp)
{
  const char *content_length;
  char *p;
  long llen;
  int len;
  int c;

  content_length = getenv("CONTENT_LENGTH");
  if (content_length == NULL)
    escm_bind(lang, "QUERY_STRING", NULL, outp);
  else {
    if (lang->bind_prefix) fputs(lang->bind_prefix, outp);
    put_variable(lang, "QUERY_STRING", outp);
    if (lang->bind_infix) fputs(lang->bind_infix, outp);
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
    if (lang->bind_suffix) fputs(lang->bind_suffix, outp);
    fputc('\n', outp);
  }
}
void
escm_init(const struct escm_lang *lang, FILE *outp)
{
  if (lang->init) {
    fputs(lang->init, outp);
    fputc('\n', outp);
  }
  /* set useful global variables if the language is scheme. */
  escm_bind(lang, "*escm-version*", PACKAGE " " VERSION, outp);
  if (!escm_is_cgi()) {
    escm_bind(lang, "GATEWAY_INTERFACE", NULL, outp);
  } else {
    const char *method;
    int i;
    method = getenv("REQUEST_METHOD");
    if (method[0] == 'P') cgi_post(lang, outp);
    else escm_bind(lang, "QUERY_STRING", getenv("QUERY_STRING"), outp);
    for (i = 0; i < SizeOfArray(env_to_bind); i++) {
      escm_bind(lang, env_to_bind[i], getenv(env_to_bind[i]), outp);
    }
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
void
escm_preproc(const struct escm_lang *lang, FILE *inp, FILE *outp)
{
  int in_string = FALSE;
  enum {
    LITERAL,
    CODE,
    DISPLAY,
  } state = LITERAL;
  int c;
  const char *p, *q;
  int str_keep_lineno = 0;
  int tag_keep_lineno = 0;

  fputs(lang->literal_prefix, outp);
  while ((c = getc(inp)) != EOF) {
    if (c == '\n') escm_lineno++;
    if (state == LITERAL) {
      if (c == '<') {
	c = getc(inp);
	if (c != '?') {
	  fputc('<', outp);
	  if (c == EOF) break;
	  ungetc(c, inp);
	  continue;
	}
	p = lang->name;
	while ((c = getc(inp)) == *p)
	  p++;
	if (isspace(c)) {
	  if (*p == '\0') {
	    state = DISPLAY;
	    fputs(lang->literal_suffix, outp);
	    fputc('\n', outp);
	    fputs(lang->display_prefix, outp);
	    tag_keep_lineno = escm_lineno;
	    if (c == '\n') escm_lineno++;
	    continue;
	  } else if (*p == ':') {
	    state = CODE;
	    fputs(lang->literal_suffix, outp);
	    fputc('\n', outp);
	    tag_keep_lineno = escm_lineno;
	    if (c == '\n') escm_lineno++;
	    continue;
	  }
	}
	fputc('<', outp);
	fputc('?', outp);
	for (q = lang->name; q < p; q++)
	  fputc(*q, outp);
	if (c == EOF) break;
	ungetc(c, inp);
      } else {
	if (c == '\\' || c == '\"') {
	  fputc('\\', outp);
	  fputc(c, outp);
	} else if (c == '\n') {
	  escm_lineno++;
	  fputc('\\', outp);
	  fputc('n', outp);
	  fputs(lang->literal_suffix, outp);
	  fputc('\n', outp);
	  fputs(lang->literal_prefix, outp);
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
	    if (state == DISPLAY) fputs(lang->display_suffix, outp);
	    fputc('\n', outp);
	    fputs(lang->literal_prefix, outp);
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
  if (state == LITERAL) fputs(lang->literal_suffix, outp);
  else {
    if (in_string) {
      fputc('\"', outp);
      escm_lineno = str_keep_lineno;
      escm_error("unterminated string");
    }
    if (state == DISPLAY) fputs(lang->display_suffix, outp);
    escm_lineno = tag_keep_lineno;
    escm_error("unterminated instruction");
  }
  fputc('\n', outp);
}
/* end of escm.c */
