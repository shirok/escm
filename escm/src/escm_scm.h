/************************************************
 * escm_scm.h - Scheme dependent part of escm
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 ***********************************************/
#ifndef ESCM_SCM_H
#define ESCM_SCM_H

#include "escm.h"
extern struct escm_lang lang_scm;
extern const char *scm_interp;
void define_string(const char *varname, const char *value, FILE *outp);
/* void define_bool(const char *varname, int bool, FILE *outp); */
#endif /* ESCM_SCM_H */
