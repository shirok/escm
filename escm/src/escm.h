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

#ifndef FILE
#include <stdio.h>
#endif /* FILE */

struct escm_form_two {
  char *prefix;
  char *suffix;
};
struct escm_form_three {
  char *prefix;
  char *infix;
  char *suffix;  
};

/* language information structure */
struct escm_lang {
  char *name;            /* the name space */
  char *backend;
  struct escm_form_two literal;
  struct escm_form_two display;
  struct escm_form_three bind;
  char *nil;
  char *newline;
  char *init;            /* initialization code */
  char *finish;          /* finalization code */
};

extern int escm_array_sep;

void escm_putc(int c, FILE *outp);
void escm_puts(const char *str, FILE *outp);
void escm_puts_array(char *const *arr, FILE *outp);

void escm_init(const struct escm_lang *lang, FILE *outp);
void escm_finish(const struct escm_lang *lang, FILE *outp);

void escm_bind(const struct escm_lang *lang, const char *var, const char *val, FILE *outp);
void escm_bind_array(const struct escm_lang *lang, const char *var, char *const *arr, FILE *outp);

/* converter */
int escm_preproc(const struct escm_lang *lang, FILE *inp, FILE *outp);

#endif /* ESCM_H */
