/*******************************************************
 * filter.c - main function for the filter tool
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
#endif /* HAVE_STDLIB_H */

#if !defined(HAVE_GETOPT_LONG) && defined(HAVE_UNISTD_H)
#  include <unistd.h>
#else
#  include <getopt.h>
#endif /* !defined(HAVE_GETOPT_LONG) && defined(HAVE_UNISTD_H) */
#if !defined(HAVE_GETOPT_LONG)
#  define getopt_long(a, b, c, d, e) getopt(a, b, c)
#endif /* !defined(HAVE_GETOPT_LONG) */

#include "escm.h"

#define EOPT_SIZE 16 /* enough? */

struct escm_lang * parse_lang(const char *name);

#define XERROR(msg) escm_error(argv[0], msg)

/* main function
 */
int
main(int argc, char **argv)
{
  int i, c;
  FILE *inp;
  FILE *outp = stdout;
  int process_flag = TRUE;
  char *lang_name = NULL;
  char *interp = NULL;
  int n_expr = 0;
  char *expr[EOPT_SIZE];

#define OPTSTR "Ee:i:l:"
#if defined(HAVE_GETOPT_LONG)
  int long_idx;
  int help_flag = FALSE;
  const struct option long_opt[] = {
    { "no-eval", 0, NULL, 'E' },
    { "eval", 1, NULL, 'e', },
    { "interp", 1, NULL, 'i', },
    { "language", 1, NULL, 'l', },
    { "help", 0, &help_flag, TRUE, },
    { "version", 0, &help_flag, FALSE, },
    { NULL, 0, NULL, 0, }
  };
#endif /* defined(HAVE_GETOPT_LONG) */
  struct escm_lang *lang = NULL;

  /* process options */
  for (;;) {
    c = getopt_long(argc, argv, OPTSTR, long_opt, &long_idx);
    if (c == -1) break;
    switch (c) {
#if defined(HAVE_GETOPT_LONG)
    case 0:
      if (help_flag) {
	printf(
"Usage: %s [OPTION] ... FILE ...\n"
"Preprocess embedded scheme code in documents.\n"
"\n"
"  -E, --no-eval                convert documents into code\n"
"  -e, --eval=EXPR              evaluate an expression\n"
"  -i, --interp='PROG ARG ...'  invoke an interpreter as backend\n"
"  -l, --language=LANG          choose the interpreter language\n"
"      --help                   print this message and exit\n"
"      --version                print version information and exit\n",
	       argv[0]);
	printf("\nReport bugs to <%s>\n", PACKAGE_BUGREPORT);
      } else {
	printf("%s - developers' version of escm\n", PACKAGE_STRING);
      }
      exit(EXIT_SUCCESS);
      /* not reached */
#endif /* defined(HAVE_GETOPT_LONG) */
    case 'E':
      process_flag = FALSE;
      break;
    case 'e':
      if (n_expr == EOPT_SIZE) XERROR("too many -e options.");
      expr[n_expr] = optarg;
      n_expr++;
      break;
    case 'i':
      interp = optarg;
      break;
    case 'l':
      lang_name = optarg;
      break;
    default:
#if defined(HAVE_GETOPT_LONG)
      printf("Try `%s --help' for more information.\n", argv[0]);
#else /* !defined(HAVE_GETOPT_LONG) */
      fprintf(stderr, "Usage: %s [-E] [-e EXPR] [-i \"PROG ARG ...\"]\n       [-l LANG] FILE ...\n", argv[0]);
      fprintf(stderr, "%s - experimental version of escm\n", PACKAGE_STRING);
#endif /* defined(HAVE_GETOPT_LONG) */
      exit(EXIT_FAILURE);
      /* not reached */
    }
  }

  /* select the language if necessary. */
  if (lang_name) {
    lang = parse_lang(lang_name);
  } else {
    lang_name = getenv("ESCM_DEFAULT");
    if (!lang_name) lang_name = "scm"; /* the default language */
    lang = parse_lang(lang_name);
    if (!interp) interp = getenv("ESCM_BACKEND");
  }
  if (lang == NULL) XERROR("invalid language configuration");
  if (!interp) interp = lang->backend;

  /* invoke the interpreter. */
  if (process_flag) outp = popen(interp, "w");
  if (outp == NULL) XERROR(NULL);

  /* initialization */
  escm_init(lang, outp);
  escm_bind(lang, "escm_version", PACKAGE " " VERSION, outp);
  if (optind == argc) {
    escm_bind(lang, "escm_input_file", NULL, outp);
  } else {
    escm_bind_array(lang, "escm_input_file", argv + optind, outp);
  }

  if (process_flag) {
    escm_bind(lang, "escm_interpreter", interp, outp);
  } else {
    escm_bind(lang, "escm_interpreter", NULL, outp);
  }

  /* evaluate the expressions specified in options */
  for (i = 0; i < n_expr; i++) {
    fputs(expr[i], outp);
    fputc('\n', outp);
  }

  if (optind == argc) {
    if (!escm_preproc(lang, stdin, outp)) XERROR("unterminated instruction");
  } else {
    for (i = optind; i < argc; i++) {
      inp = fopen(argv[i], "r");
      if (inp == NULL) XERROR(NULL);
      if (!escm_preproc(lang, inp, outp)) XERROR("unterminated instruction");
      fclose(inp);
    }
  }

  /* finalization */
  escm_finish(lang, outp);

  /* close the pipe. */
  if (process_flag) {
    if (pclose(outp) == -1) XERROR(NULL);
  }
  return 0;
}
/* end of filter.c */
