/* escm.c - XML preprocessor
 * $Id$
 *
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */

#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include "escm.h"

/* The last resort version of escm_puts() */
#if !HAVE_ESCM_PUTS && !defined(escm_puts)
void
escm_puts(const char* str, escm_outp_t *fp)
{
  const char *p = str;
  while (*p) {
    escm_putc(*p, fp);
    p++;
  }
}
#endif /* escm_puts */

/* escm_puts_string(str, pout)
 */
void
escm_puts_string(const char *p, escm_outp_t *out)
{
  escm_putc('\"', out);
  for (/**/; *p; p++) {
    if (*p == '\\' || *p == '\"') escm_putc('\\', out);
    escm_putc(*p, out);
  }
  escm_putc('\"', out);
}
/* escm_puts_varname(varname, pout, lang->varname)
 */
void
escm_puts_varname(const char *p, escm_outp_t *out, enum escm_varname type)
{
  int flag = TRUE;
  for (/**/; *p; p++) {
    if (type == ESCM_VN_HYPHEN && *p == '_') escm_putc('-', out);
    else if (type == ESCM_VN_UPPER) escm_putc(toupper(*p), out);
    else if (type == ESCM_VN_LOWER) escm_putc(tolower(*p), out);
    else if (type == ESCM_VN_TITLE) {
      if (*p == '_') {
	flag = TRUE;
      } else if (flag) {
	escm_putc(toupper(*p), out);
	flag = FALSE;
      } else {
	escm_putc(tolower(*p), out);
      }
    } else escm_putc(*p, out);
  }
}
/* escm_define(lang, varname, expr, pout, string_p)
 */
void
escm_define(struct escm_lang *lang, const char *varname, const char *expr,
	    escm_outp_t *pout, int string_p)
{
  if (lang->define_prefix) escm_puts(lang->define_prefix, pout);
  escm_puts_varname(varname, pout, lang->varname);
  if (lang->define_infix) escm_puts(lang->define_infix, pout);
  if (expr == NULL) {
    escm_puts(lang->false, pout);
  } else if (string_p) {
    escm_puts_string(expr, pout);
  } else escm_puts(expr, pout);
  if (lang->define_suffix) escm_puts(lang->define_suffix, pout);
  escm_putc('\n', pout);
}


/* escm_preproc(lang, inp, outp, c, prefix, suffix, func)
 */
void
escm_preproc(struct escm_lang *lang, struct escm_style *style,
	     escm_inp_t *in, escm_outp_t *out, int c)
{
  enum escm_state state = ESCM_LITERAL;
  int in_string_p = FALSE;

  if (c == EOF) {
    c = escm_getc(in);
    if (c == EOF) return;
  }
  escm_puts(lang->literal_prefix, out);

  for (/**/; c != EOF; c = escm_getc(in)) {
    if (state == ESCM_LITERAL) {
      if (c == '<') {
	c = escm_getc(in);
	if (c != style->prefix_char) {
	  escm_putc('<', out);
	  if (c == EOF) break;
	  escm_ungetc(c, in);
	} else {
	  if (! style->change_state(lang, in, out, &state)) break;
	  in_string_p = FALSE;
	}
      } else if (c == '\n') {
	escm_putc('\\', out);
	escm_putc('n', out);
      } else {
	if (c == '\\' || c == '\"') escm_putc('\\', out);
	escm_putc(c, out);
      }
    } else { /* state == ESCM_CODE || state == ESCM_DISPLAY */
      if (in_string_p) {
	if (c == '\"') {
	  in_string_p = FALSE;
	  escm_putc('\"', out);
	} else if (c == '\\') {
	  c = escm_getc(in);
	  if (c == EOF) {
	    escm_putc('\\', out);
	    /* May cause an error to the backend interpreter */
	    break;
	  } else {
	    escm_putc('\\', out);
	    escm_putc(c, out);
	  }
	} else {
	  escm_putc(c, out);
	}
      } else {
	if (c == style->suffix_char) {
	  c = escm_getc(in);
	  if (c == '>') {
	    if (state == ESCM_DISPLAY) escm_puts(lang->display_suffix, out);
	    escm_putc('\n', out); /* maybe in a comment */
	    state = ESCM_LITERAL;
	    escm_puts(lang->literal_prefix, out);
	  } else {
	    escm_putc(style->suffix_char, out);
	    if (c == EOF) break;
	    escm_putc(c, out);
	  }
	} else if (c == '\"') {
	  escm_putc('\"', out);
	  in_string_p = TRUE;
	} else if (c == '\\') {
	  c = escm_getc(in);
	  if (c == EOF) {
	    escm_putc('\\', out);
	    break;
	  } else {
	    escm_putc('\\', out);
	    escm_putc(c, out);
	  }
	} else {
	  escm_putc(c, out);
	}
      }
    }
  }
  if (state == ESCM_LITERAL) escm_puts(lang->literal_suffix, out);
  else if (state == ESCM_DISPLAY) escm_puts(lang->display_suffix, out);
  escm_putc('\n', out);
}

#ifdef ESCM_ENABLE_XMLPI
static int
xmlpi_change_state(struct escm_lang *lang, escm_inp_t *in, escm_outp_t *out,
		   enum escm_state *state)
{
  int c = EOF;
  const char *p, *q;

  for (p = lang->name; *p; p++) {
    c = escm_getc(in);
    if (c != *p) break;
  }
  if (*p != '\0') {
    escm_puts("<?", out);
    for (q = lang->name; q < p; q++)
      escm_putc(*q, out);
  } else {
    c = escm_getc(in);
    if (isspace(c)) {
      escm_puts(lang->literal_suffix, out);
      escm_putc('\n', out);
      *state = ESCM_CODE;
      return TRUE;
    } else if (c != ':' || lang->display == NULL) {
      escm_puts("<?", out);
      escm_puts(lang->name, out);
    } else { /* c == ':' */
      for (p = lang->display; *p; p++) {
	c = escm_getc(in);
	if (c != *p) break;
      }
      if (*p != '\0') {
	escm_puts("<?", out);
	escm_puts(lang->name, out);
	escm_putc(':', out);
	for (q = lang->display; q < p; q++)
	  escm_putc(*q, out);
      } else {
	c = escm_getc(in);
	if (isspace(c)) {
	  escm_puts(lang->literal_suffix, out);
	  escm_putc('\n', out);
	  *state = ESCM_DISPLAY;
	  escm_puts(lang->display_prefix, out);
	  return TRUE;
	} else {
	  escm_puts("<?", out);
	  escm_puts(lang->name, out);
	  escm_putc(':', out);
	  escm_puts(lang->display, out);
	}
      }
    }
  }
  if (c == EOF) return FALSE;
  escm_ungetc(c, in);
  return TRUE;
}
struct escm_style escm_xmlpi_style = {
  '?', /* prefix_char */
  '?', /* suffix_char */
  xmlpi_change_state, /* change_state */
};
#endif /* ESCM_ENABLE_XMLPI */
#ifdef ESCM_ENABLE_ARCHAIC
static int
archaic_change_state(struct escm_lang *lang, escm_inp_t *in, escm_outp_t *out,
		     enum escm_state *state)
{
  int c;

  c = escm_getc(in);
  if (c == '=') {
    escm_puts(lang->literal_suffix, out);
    escm_putc('\n', out);
    *state = ESCM_DISPLAY;
    escm_puts(lang->display_prefix, out);
  } else if (c == EOF) {
    escm_putc('<', out);
    escm_putc('?', out);
    return FALSE;
  } else {
    escm_ungetc(c, in);
    escm_puts(lang->literal_suffix, out);
    escm_putc('\n', out);
    *state = ESCM_CODE;
  }
  return TRUE;
}
struct escm_style escm_archaic_style = {
  '?', /* prefix_char */
  '!', /* suffix_char */
  archaic_change_state, /* change_state */
};
#endif /* ESCM_ENABLE_ARCHAIC */

#ifdef ESCM_ENABLE_SCRIPT
static int
script_change_state(struct escm_lang *lang, escm_inp_t *in, escm_outp_t *out,
		    enum escm_state *state)
{
  int c;

  c = escm_getc(in);
  if (c == '%') {
    escm_putc('<', out);
    escm_putc('%', out);
  } else if (c == '=') {
    escm_puts(lang->literal_suffix, out);
    escm_putc('\n', out);
    *state = ESCM_DISPLAY;
    escm_puts(lang->display_prefix, out);
  } else if (c == EOF) {
    escm_putc('<', out);
    escm_putc('%', out);
    return FALSE;
  } else {
    escm_ungetc(c, in);
    escm_puts(lang->literal_suffix, out);
    escm_putc('\n', out);
    *state = ESCM_CODE;
  }
  return TRUE;
}
struct escm_style escm_script_style = {
  '%', /* prefix_char */
  '%', /* suffix_char */
  script_change_state, /* change_state */
};
#endif /* ESCM_ENABLE_SCRIPT */
/* end of escm.c */
