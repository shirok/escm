/*******************************************************
 * cgimain.c - main function for the CGI tool
 * $Id$
 * Copyright (c) 2003-2004 TAGA Yoshitaka <tagga@tsuda.ac.jp>
 ********************************************************/
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#if defined(HAVE_STDLIB_H)
#  include <stdlib.h>
#else
#  define EXIT_SUCCESS 0
#  define EXIT_FAILURE 1
   void exit(int status);
   char *getenv(const char* name);
#endif /* HAVE_STDLIB_H */

#if defined(HAVE_UNISTD_H)
#  include <unistd.h>
#else
#  include <getopt.h>
#endif /* defined(HAVE_UNISTD_H) */

#include "escm.h"
#include "escmcgi.h"

extern struct escm_lang deflang;

/* xerror(message)
 */
static void
xerror(const char *mesg)
{
  fputs("escm(cgi): ", stderr);
  fputs(mesg, stderr);
  exit(EXIT_FAILURE);
}

/* main function
 */
int
main(int argc, char **argv)
{
  int c;
  int process_flag = TRUE;
  char *infile = NULL;
  FILE *inp;
  FILE *outp = stdout;

  for (;;) {
    c = getopt(argc, argv, "E");
    if (c == -1) break;
    switch (c) {
    case 'E':
      process_flag = FALSE;
      break;
    default:
      fprintf(stderr, "Usage: %s [-E] [FILE]\n", argv[0]);
      exit(EXIT_FAILURE);
      /* not reached */
    }
  }

  if (process_flag) {
    outp = popen(ESCM_BACKEND, "w");
    if (outp == NULL)
      xerror("can't invokde the backend " ESCM_BACKEND ".\n");
  } else {
    fputs("Content-type: text/plain\r\n\r\n", outp);
  }

  /* decide what file to open. */
  if (optind == argc) {
    infile = getenv("PATH_TRANSLATED");
  } else if (optind + 1 == argc) {
    infile = argv[optind];
  }
  if (infile == NULL)
    xerror("don't know which file to open.\n");

  escm_init(&deflang, outp);
  escm_bind(&deflang, "escm_version", PACKAGE " " VERSION "(cgi)", outp);
  escm_bind(&deflang, "escm_input_file", infile, outp);
  escm_bind(&deflang, "escm_interpreter", ESCM_BACKEND, outp);
  if (!escm_query_string(&deflang, outp))
    xerror("inconsistent environment variables.\n");
  inp = fopen(infile, "r");
  if (inp == NULL)
    xerror("can't open the input file.\n");

  escm_skip_shebang(inp);
  escm_add_header(&deflang, inp, outp);

  if (!escm_preproc(&deflang, inp, outp))
    xerror("unterminated instruction.\n");
  fclose(inp);

  escm_finish(&deflang, outp);

  if (process_flag) {
    if (pclose(outp) == -1)
      xerror("can't closee the pipe.\n");
  }
  return 0;
}
/* end of cgimain.c */
