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

/* main function
 */
int
main(void)
{
  int i, ret;
  const char *path;
  FILE *inp = NULL;
  FILE *outp = stdout;

  if (! escm_is_cgi()) {
    escm_error(PACKAGE "-handler", "must be an action handler.");
  }
  path = getenv("PATH_TRANSLATED");
  if (path == NULL) {
    escm_error(PACKAGE "-handler", "must be an action handler.");
  }

  escm_stderr2stdout();
  escm_html_header();
  outp = popen(scm_interp, "w");
  if (outp == NULL)
    escm_error(PACKAGE, "Can't invoke the interpreter.");

  escm_init(&lang_scm, outp);

  /* set useful global variables if the language is scheme. */
  define_string("*escm-version*", PACKAGE " " VERSION, outp);
  define_string("*escm-interpreter*", scm_interp, outp);
  define_string("*escm-output-file*", NULL, outp);
  define_string("*escm-input-file*", path, outp);

  inp = fopen(path, "r");
  if (inp == NULL) escm_error(PACKAGE "-handler", NULL);
  ret = escm_preproc(&lang_scm, inp, outp);
  if (! ret) escm_error(PACKAGE "-handler", "Syntax error.");
  escm_finish(&lang_scm, outp);

  ret = pclose(outp);
  if (ret == -1) escm_error(PACKAGE, NULL);
  return 0;
}
/* end of handler.c */
