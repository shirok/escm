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
#include "misc.h"

/* Used to make error messages. */
const char *escm_prog = NULL;
const char *escm_file = NULL;

/* put_string(str, outp) - escape str and put it.
 * This function is not used in this file, but will be useful.
 */
static void
put_string(const char *str, FILE *outp)
{
  const char *p = str;
  fputc('\"', outp);
  while (*p) {
    if (*p == '\"' || *p == '\\') {
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
  fputc('\"', outp);
}
/* foo_bar => *foo-bar* */
static void
put_lisp_variable(const char *var, FILE *outp)
{
  const char* p;
  fputc('*', outp);
  for (p = var; *p; p++) {
    if (*p == '_' || isspace(*p)) {
      fputc('-', outp);
    } else {
      fputc(*p, outp);
    }
  }
  fputc('*', outp);
}
/* foo_bar => foo_bar */
static void
put_lower_variable(const char *var, FILE *outp)
{
  const char* p;
  p = var;
  if (! isalpha(*p)) {
    fputc('_', outp);
  } else {
    fputc(tolower(*p), outp);
  }
  for (p = var + 1; *p; p++) {
    if (! isalnum(*p)) {
      fputc('_', outp);
    } else {
      fputc(tolower(*p), outp);
    }
  }
}
/* foo_bar => FOO_BAR */
static void
put_upper_variable(const char *var, FILE *outp)
{
  const char* p;
  p = var;
  if (! isalpha(*p)) {
    fputc('_', outp);
  } else {
    fputc(toupper(*p), outp);
  }
  for (p = var + 1; *p; p++) {
    if (! isalnum(*p)) {
      fputc('_', outp);
    } else {
      fputc(toupper(*p), outp);
    }
  }
}
/* foo_bar => FooBar */
static void
put_title_variable(const char *var, FILE *outp)
{
  const char* p;
  int flag = FALSE;
  p = var;
  if (! isalpha(*p)) {
    fputc('_', outp);
  } else {
    if (flag) {
      fputc(tolower(*p), outp);
    } else {
      fputc(toupper(*p), outp);
      flag = FALSE;
    }
  }
  for (p = var + 1; *p; p++) {
    if (! isalpha(*p)) {
      flag = FALSE;
    } else {
      if (flag) {
	fputc(tolower(*p), outp);
      } else {
	fputc(toupper(*p), outp);
	flag = FALSE;
      }
    }
  }
}

/* put_variable(lang, var, outp) - put var as a variable name.
 */
static void
put_variable(const struct escm_lang *lang, const char *var, FILE *outp)
{
  if (lang->id_type == ESCM_ID_LISP) {
    put_lisp_variable(var, outp);
  } else if (lang->id_type == ESCM_ID_LOWER) {
    put_lower_variable(var, outp);
  } else if (lang->id_type == ESCM_ID_UPPER) {
    put_upper_variable(var, outp);
  } else {
    put_title_variable(var, outp);
  }
}
/* escm_header(lang, inp, outp)
 */
void
escm_header(const struct escm_lang *lang, FILE *inp, FILE *outp)
{
  int c;

  c = fgetc(inp);
  if (c == '<') {
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
    cgi_header_flag = TRUE;
  }
  ungetc(c, inp);
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
    XERROR("inconsistent environment");
  else {
    if (lang->bind.prefix) fputs(lang->bind.prefix, outp);
    put_variable(lang, "escm_query_string", outp);
    if (lang->bind.infix) fputs(lang->bind.infix, outp);
    llen = strtol(content_length, &p, 10);
    if (*p == '\0') {
      fputc('\"', outp);
      len = (int) llen;
      while ((c = getc(stdin)) != EOF && len-- > 0) {
	/* better replace the next line with an error check. */
	if (c == '\"') fputc('\\', outp);
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
/* escm_preproc(&lang, inp, outp) - the preprocessor.
 */
#define push_char(c) {\
 if (stackptr == 63) goto empty;\
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
  } state = LITERAL;
  int c;
  const char *p;
  char stack[64] = "<?";
  int stackptr = 0, i = 0;

  fputs(lang->literal.prefix, outp);
  while ((c = getc(inp)) != EOF) {
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
	  continue;
	}
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
	  if (lang->newline) {
	    fputs(lang->literal.suffix, outp);
	    fputs(lang->newline, outp);
	  } else {
	    fputc('\\', outp);
	    fputc('n', outp);
	    fputs(lang->literal.suffix, outp);
	  }
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
	    fputc('\n', outp);
	    fputs(lang->literal.prefix, outp);
	    state = LITERAL;
	    continue;
	  } else fputc('?', outp);
	} else if (c == '\"') {
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
      XERROR("unterminated string");
    }
    if (state == DISPLAY) fputs(lang->display.suffix, outp);
    XERROR("unterminated instruction");
  }
  fputc('\n', outp);
}
/* end of escm.c */
