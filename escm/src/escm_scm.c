/*************************************************
 * escm_scm.c - Scheme dependent part of escm
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 *************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */
#include "escm.h"

struct escm_lang lang_scm = {
  "scm:d", /* name */
  "(display \"", /* literal_prefix */
  "\")", /* literal_suffix */
  "(display ", /* display_prefix */
  ")", /* display_suffix */
  NULL, /* init */
  NULL, /* finish */
};

#ifdef ESCM_SCM
const char * scm_interp = ESCM_SCM;
#else
const char * scm_interp = "gosh -b";
#endif /* ESCM_SCM */

/* define_string(varname, value, oupt) - (define varname value)
 * where value is a string.
 */
void
define_string(const char *varname, const char *value, FILE *outp)
{
  fputs("(define ", outp);
  fputs(varname, outp);
  fputs(" ", outp);
  if (value) escm_put_string(value, outp);
  else fputs("#f", outp);
  fputs(")\n", outp);
}
#if 0
/* define_bool(varname, value, outp) - (define varname value)
 * where value is a boolean value.
 */
void
define_bool(const char *varname, int bool, FILE *outp)
{
  fputs("(define ", outp);
  fputs(varname, outp);
  fputs(" ", outp);
  fputs(bool? "#t" : "#f", outp);
  fputs(")\n", outp);
}
#endif /* 0 */
/* end of escm_scm.c */
