/* escm.h - preprocess XML with scheme or another
 * interpreter language.
 * $Id$
 * Copyright (c) 2003-2004 TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifndef ESCM_H
#  define ESCM_H 1

#ifndef TRUE
#define FALSE 0
#define TRUE (!FALSE)
#endif /* TRUE */

#include <stdio.h>

struct escm_form_two {
  char *prefix;
  char *suffix;
};
struct escm_form_three {
  char *prefix;
  char *infix;
  char *suffix;  
};

enum ESCM_ID_TYPE {
  ESCM_ID_LISP,
  ESCM_ID_LOWER,
  ESCM_ID_UPPER,
  ESCM_ID_TITLE,
};

/* language information structure */
struct escm_lang {
  char *name;            /* the name space */
  enum ESCM_ID_TYPE id_type;
  char *backend;
  struct escm_form_two literal;
  struct escm_form_two display;
  struct escm_form_three bind;
  struct escm_form_three assign;
  char *nil;
  char *newline;
  char *init;            /* initialization code */
  char *finish;          /* finalization code */
};

void escm_header(const struct escm_lang *lang, FILE *inp, FILE *outp);
void escm_init(const struct escm_lang *lang, FILE *outp);
void escm_finish(const struct escm_lang *lang, FILE *outp);
void escm_preproc(const struct escm_lang *lang, FILE *inp, FILE *outp);
void escm_bind(const struct escm_lang *lang, const char *var, const char *val, FILE *outp);
void escm_assign(const struct escm_lang *lang, const char *var, const char *val, FILE *outp);

void escm_error(const char *message);

void escm_bind_query_string(const struct escm_lang *lang, FILE *outp);
#endif /* ESCM_H */
