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

/* the default escm_lang object.
 */
struct escm_lang lang_scm = {
  "scm:d", /* name */
  "(display \"", /* literal_prefix */
  "\")", /* literal_suffix */
  "(display ", /* display_prefix */
  ")", /* display_suffix */
  "(define ", /* define_prefix */
  " ", /* define_infix */
  ")", /* define_suffix */
  1, /* use_hyphen */
  "#t", /* true */
  "#f", /* false */
  NULL, /* init */
  NULL, /* finish */
};

static char * env_to_bind[] = {
  "GATEWAY_INTERFACE",
  "HTTP_ACCEPT_LANGUAGE",
  "HTTP_ACCEPT_CHARSET",
  "HTTP_COOKIE",
  "HTTP_HOST",
  "HTTP_REFERER",
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
/* escm_define(lang, var, val, outp) - define var to val in lang
 */
void
escm_define(const struct escm_lang *lang, const char *var, const char *val, FILE *outp)
{
  fputs(lang->define_prefix, outp);
  put_variable(lang, var, outp);
  fputs(lang->define_infix, outp);
  if (val == NULL) fputs(lang->false, outp);
  else put_string(val, outp);
  fputs(lang->define_suffix, outp);
  fputc('\n', outp);
}
/* escm_init(&lang, outp) - initialize the backend interpreter.
 */
static void
cgi_post(struct escm_lang *lang, FILE *outp)
{
  const char *content_length;
  char *p;
  long llen;
  int len;
  int c;

  content_length = getenv("CONTENT_LENGTH");
  if (content_length == NULL)
    escm_define(lang, "QUERY_STRING", NULL, outp);
  else {
    fputs(lang->define_prefix, outp);
    put_variable(lang, "QUERY_STRING", outp);
    fputs(lang->define_infix, outp);
    llen = strtol(content_length, &p, 10);
    if (*p == '\0') {
      fputc('\"', outp);
      len = (int) llen;
      while ((c = getc(stdin)) != EOF && len-- > 0) {
	fputc(c, outp);
      }
      fputc('\"', outp);
    } else {
      fputs(lang->false, outp);
    }
    fputs(lang->define_suffix, outp);
  }
}
void
escm_init(struct escm_lang *lang, FILE *outp)
{
  if (lang->init) fputs(lang->init, outp);
  /* set useful global variables if the language is scheme. */
  escm_define(lang, "*escm-version*", PACKAGE " " VERSION, outp);
  if (!escm_is_cgi()) {
    escm_define(lang, "GATEWAY_INTERFACE", NULL, outp);
  } else {
    const char *method;
    int i;
    method = getenv("REQUEST_METHOD");
    if (method[0] == 'P') cgi_post(lang, outp);
    else escm_define(lang, "QUERY_STRING", getenv("QUERY_STRING"), outp);
    for (i = 0; i < SizeOfArray(env_to_bind); i++) {
      escm_define(lang, env_to_bind[i], getenv(env_to_bind[i]), outp);
    }
  }
}
/* escm_finish(&lang, outp) - finalize the backend interpreter.
 */
void
escm_finish(struct escm_lang *lang, FILE *outp)
{
  if (lang->finish) fputs(lang->finish, outp);
}
/* escm_preproc(&lang, inp, outp) - the preprocessor.
 */
int
escm_preproc(struct escm_lang *lang, FILE *inp, FILE *outp)
{
  int ret = TRUE;
  int in_string = FALSE;
  enum {
    LITERAL,
    CODE,
    DISPLAY,
  } state = LITERAL;
  int c;
  const char *p, *q;

  fputs(lang->literal_prefix, outp);
  while ((c = getc(inp)) != EOF) {
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
	    continue;
	  } else if (*p == ':') {
	    state = CODE;
	    fputs(lang->literal_suffix, outp);
	    fputc('\n', outp);
	    continue;
	  }
	}
	fputc('<', outp);
	fputc('?', outp);
	for (q = lang->name; q < p; q++)
	  fputc(*q, outp);
	if (c == EOF) { /* a broken file */
	  ret = FALSE;
	  break;
	}
	fputc(c, outp);
      } else {
	if (c == '\\' || c == '\"') {
	  fputc('\\', outp);
	  fputc(c, outp);
	} else if (c == '\n') {
	  fputc('\\', outp);
	  fputc('n', outp);
	} else {
	  fputc(c, outp);	  
	}
      }
    } else {
      if (in_string) {
	if (c == '\\') {
	  fputc('\\', outp);
	  c = getc(inp);
	  if (c == EOF) { /* a broken file */
	    fputc('\\', outp);
	    break;
	  }
	} else if (c == '\"') in_string = FALSE;
	fputc(c, outp);
      } else {
	if (c == '?') {
	  c = getc(inp);
	  if (c == '>') {
	    if (state == DISPLAY) fputs(lang->display_suffix, outp);
	    fputc('\n', outp);
	    fputs(lang->literal_prefix, outp);
	    state = LITERAL;
	    continue;
	  } else {
	    fputc('?', outp);
	    if (c == EOF) { /* a broken file */
	      break;
	    }
	  }
	} else if (c == '\"') in_string = TRUE;
	fputc(c, outp);
      }
    }
  }
  if (state == LITERAL) fputs(lang->literal_suffix, outp);
  else {
    ret = FALSE;
    if (in_string) fputc('\"', outp);
    if (state == DISPLAY) fputs(lang->display_suffix, outp);
  }
  fputc('\n', outp);
  return ret;
}
/* end of escm.c */
