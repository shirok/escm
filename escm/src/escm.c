/************************************************
 * escm.c - preprocess XML with scheme or another
 * interpreter language.
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 ***********************************************/
#include <stdio.h>
#include <ctype.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */
#include "escm.h"

/* the default escm_lang object.
 */
struct escm_lang lang_scm = {
  "scm:d", /* name */
  "(display \"", /* literal_prefix */
  "\")", /* literal_suffix */
  "(display ", /* display_prefix */
  ")", /* display_suffix */
  "(define *", /* define_prefix */
  "* ", /* define_infix */
  ")", /* define_suffix */
  1, /* use_hyphen */
  "#f", /* null */
  NULL, /* init */
  NULL, /* finish */
};

/* escm_put_string(str, outp) - escape str and put it.
 * This function is not used in this file, but will be useful.
 */
void
escm_put_string(const char *str, FILE *outp)
{
  const char *p = str;
  fputc('\"', outp);
  while (*p) {
    if (*p == '\"' || *p == '\\') fputc('\\', outp);
    fputc(*p++, outp);
  }
  fputc('\"', outp);
}
/* escm_define(lang, var, val, outp) - define var to val in lang
 */
void
escm_define(const struct escm_lang *lang, const char *var, const char *val, FILE *outp)
{
  const char *p;

  fputs(lang->define_prefix, outp);
  for (p = var; *p; p++) {
    if (*p == '-' && !lang->use_hyphen) {
      fputc('_', outp);
    } else {
      fputc(*p, outp);
    }
  }
  fputs(lang->define_infix, outp);
  if (val == NULL) fputs(lang->null, outp);
  else escm_put_string(val, outp);
  fputs(lang->define_suffix, outp);
  fputc('\n', outp);
}

/* escm_init(&lang, outp) - initialize the backend interpreter.
 */
void
escm_init(struct escm_lang *lang, FILE *outp)
{
  if (lang->init) fputs(lang->init, outp);
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
	  fputc(c, outp);
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
