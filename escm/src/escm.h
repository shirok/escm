/* escm.h - preprocess XML with scheme or another
 * interpreter language.
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifndef ESCM_H
#  define ESCM_H 1

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
/* language information structure */
struct escm_lang {
  char *name;            /* the name space */
  int scm_p;
  char **backend;
  struct escm_form_two literal;
  struct escm_form_two display;
  struct escm_form_three bind;
  struct escm_form_three assign;
  char *nil;
  char *newline;
  char *init;            /* initialization code */
  char *finish;          /* finalization code */
};

/* escm_init(&lang, outp) - initialize the backend interpreter. */
void escm_init(const struct escm_lang *lang, FILE *outp);
/* escm_finish(&lang, outp) - finalize the backend interpreter. */
void escm_finish(const struct escm_lang *lang, FILE *outp);
/* escm_preproc(&lang, inp, outp) - the preprocessor. */
void escm_preproc(const struct escm_lang *lang, FILE *inp, FILE *outp);
#endif /* ESCM_H */
