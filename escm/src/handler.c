/*******************************************************
 * handler.c - main function for the action handler
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 ********************************************************/
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include "escm.h"
#include "escm_scm.h"

#ifdef ESCM_SCM
const char *scm_interp = ESCM_SCM;
#else
const char *scm_interp = "gosh -b"
#endif /* ESCM_SCM */

/* main function
 */
int
main(void)
{
  int ret;
  const char *path;
  FILE *inp = NULL;
  FILE *outp = stdout;

  if (! escm_is_cgi()) {
    escm_error("It does not seem to be an action handler.");
  }
  path = getenv("PATH_TRANSLATED");
  if (path == NULL) {
    escm_error("It does not seem to be an action handler.");
  }

  escm_stderr2stdout();
  escm_html_header();
  outp = popen(scm_interp, "w");
  if (outp == NULL)
    escm_error("Can't invoke the interpreter.");

  escm_init(&lang_scm, outp);

  /* set useful global variables if the language is scheme. */
  escm_define(&lang_scm, "escm-version", PACKAGE " " VERSION, outp);
  escm_define(&lang_scm, "escm-interpreter", scm_interp, outp);
  escm_define(&lang_scm, "escm-output-file", NULL, outp);
  escm_define(&lang_scm, "escm-input-file", path, outp);

  inp = fopen(path, "r");
  if (inp == NULL) escm_error(NULL);
  ret = escm_preproc(&lang_scm, inp, outp);
  if (! ret) escm_error("Syntax error.");
  escm_finish(&lang_scm, outp);

  ret = pclose(outp);
  if (ret == -1) escm_error(NULL);
  return 0;
}
/* end of handler.c */
