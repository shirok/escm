/************************************************
 * escm.c - preprocess XML with scheme or another
 * interpreter language.
 * $Id$
 * Copyright (c) 2003-2004 TAGA Yoshitaka <tagga@tsuda.ac.jp>
 ***********************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include <ctype.h>
#include "escm.h"

/* escm_puts(str, outp) - escape str and put it.
 */
void
escm_puts(const char *str, FILE *outp)
{
  const char *p = str;
  fputc('"', outp);
  while (*p) {
    if (*p == '"' || *p == '\\') {
      fputc('\\', outp);
      fputc(*p, outp);
    } else if (*p == '\n') {
      fputc('\\', outp);
      fputc('n', outp);
    } else {
      fputc(*p, outp);
    }
    p++;
  }
  fputc('"', outp);
}
/* escm_bind(lang, var, val, outp) - bind var to val in lang
 */
void
escm_bind(const struct escm_lang *lang, const char *var, const char *val, FILE *outp)
{
  if (lang->bind.prefix) fputs(lang->bind.prefix, outp);
  fputs(var, outp);
  if (lang->bind.infix) fputs(lang->bind.infix, outp);
  if (val == NULL) fputs(lang->nil, outp);
  else escm_puts(val, outp);
  if (lang->bind.suffix) fputs(lang->bind.suffix, outp);
  fputc('\n', outp);
}
/* escm_assign(lang, var, val, outp) - assign var to val in lang
 */
void
escm_assign(const struct escm_lang *lang, const char *var, const char *val, FILE *outp)
{
  if (lang->assign.prefix) fputs(lang->assign.prefix, outp);
  fputs(var, outp);
  if (lang->assign.infix) fputs(lang->assign.infix, outp);
  if (val == NULL) fputs(lang->nil, outp);
  else escm_puts(val, outp);
  if (lang->assign.suffix) fputs(lang->assign.suffix, outp);
  fputc('\n', outp);
}
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

/* escm_preproc(lang, inp, outp)
 */
int
escm_preproc(const struct escm_lang *lang, FILE *inp, FILE *outp)
{
  int prefixed = FALSE;
  int display = FALSE;
  int string = FALSE;
  int c;
  int i, j;
  enum {
    OUT,
    LITERAL,
    CODE,
  } state = LITERAL;

  while (state != OUT) {
    if (state == LITERAL) {
      c = fgetc(inp);
      switch (c) {
      case EOF:
	state = OUT;
	break;
      case '\n':
	if (lang->newline) {
	  if (prefixed) fputs(lang->literal.suffix, outp);
	  fputs(lang->newline, outp);
	} else {
	  if (!prefixed) fputs(lang->literal.prefix, outp);
	  fputc('\\', outp);
	  fputc('n', outp);
	  fputs(lang->literal.suffix, outp);
	}
	fputc('\n', outp);
	prefixed = FALSE;
	break;
      case '<':
	c = fgetc(inp);
	if (c != '?') {
	  if (!prefixed) {
	    fputs(lang->literal.prefix, outp);
	    prefixed = TRUE;
	  }
	  fputc('<', outp);
	  if (c == EOF) state = OUT;
	  else ungetc(c, inp);
	} else {
	  for (i = 0; lang->name[i]; i++) {
	    c = fgetc(inp);
	    if (c == EOF) goto invalid;
	    else if (c != lang->name[i]) {
	      ungetc(c, inp);
	      goto invalid;
	    }
	  }
	  c = fgetc(inp);
	  if (c == EOF) return FALSE;
	  else if (isspace(c)) {
	    display = FALSE;
	    if (prefixed) fputs(lang->literal.suffix, outp);
	    state = CODE;
	  } else if (c != ':') {
	    ungetc(c, inp);
	    goto invalid;
	  } else {
	    c = fgetc(inp);
	    if (c != 'd' || !lang->display.prefix) {
	      if (c == EOF) return FALSE;
	      if (!prefixed) {
		fputs(lang->literal.prefix, outp);
		prefixed = TRUE;
	      }
	      fputc('<', outp);
	      fputc('?', outp);
	      fputs(lang->name, outp);
	      fputc(':', outp);
	      fputc(c, outp);
	    } else {
	      display = TRUE;
	      if (prefixed) fputs(lang->literal.suffix, outp);
	      fputs(lang->display.prefix, outp);
	      state = CODE;
	    }
	  }
	}
	break;

      invalid:
	if (!prefixed) {
	  fputs(lang->literal.prefix, outp);
	  prefixed = TRUE;
	}
	fputc('<', outp);
	fputc('?', outp);
	for (j = 0; j < i; j++)
	  fputc(lang->name[j], outp);
	break;

      default:
	if (!prefixed) {
	  fputs(lang->literal.prefix, outp);
	  prefixed = TRUE;
	}
	if (c == '"' || c == '\\') fputc('\\', outp);
	fputc(c, outp);
      }
    } else {
      c = fgetc(inp);
      switch (c) {
      case EOF:
	return FALSE;
	/* not reached */
      case '\\':
	c = fgetc(inp);
	switch (c) {
	case EOF:
	  return FALSE;
	  /* not reached */
	default:
	  fputc('\\', outp);
	  fputc(c, outp);
	}
	break;
      case '"':
	fputc('"', outp);
	string = !string;
	break;
      case '?':
	if (!string) {
	  c = fgetc(inp);
	  switch (c) {
	  case EOF:
	    return FALSE;
	    /* not reached */
	  case '>':
	    prefixed = FALSE;
	    if (display) fputs(lang->display.suffix, outp);
	    fputc('\n', outp);
	    state = LITERAL;
	    break;
	  default:
	    fputc('?', outp);
	    fputc(c, outp);
	  }
	  break;
	}
      default:
	fputc(c, outp);
      }
    }
  }

  if (prefixed) fputs(lang->literal.suffix, outp);
  fputc('\n', outp);
  return TRUE;
}

/* end of escm.c */
