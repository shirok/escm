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

/* put_string(str, outp) - escape str and put it.
 */
static void
put_string(const char *str, FILE *outp)
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
  fputs(var, outp);
  if (lang->assign.infix) fputs(lang->assign.infix, outp);
  if (val == NULL) fputs(lang->nil, outp);
  else put_string(val, outp);
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

#ifdef ENABLE_CGI
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
    else put_string(p, outp);
    if (lang->setter.suffix) fputs(lang->setter.suffix, outp);
    fputc('\n', outp);
  }
  return TRUE;
}
#undef setter
#endif /* ENABLE_CGI */

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
