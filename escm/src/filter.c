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
#  void exit(int status);
#endif /* HAVE_STDLIB_H */

#if defined(HAVE_STRING_H)
#  include <string.h>
#elif defined(HAVE_STRINGS_H)
#  include <strings.h>
#endif /* HAVE_STRING_H and HAVE_STRINGS_H */

#if !defined(HAVE_GETOPT_LONG) && defined(HAVE_UNISTD_H)
#  include <unistd.h>
#else
#  include <getopt.h>
#endif /* !defined(HAVE_GETOPT_LONG) && defined(HAVE_UNISTD_H) */
#if !defined(HAVE_GETOPT_LONG)
#  define getopt_long(a, b, c, d, e) getopt(a, b, c)
#endif /* !defined(HAVE_GETOPT_LONG) */
#if !defined POSIXLY_CORRECT
# define POSIXLY_CORRECT 1
#endif /* !defined POSIXLY_CORRECT */

#include "escm.h"
#include "misc.h"
#include "fork.h"
#include "cmdline.h"

#define SizeOfArray(arr) (sizeof(arr) / (sizeof(arr[0])))

extern struct escm_lang deflang;

#ifdef ENABLE_POLYGLOT
/* defined in lang.c */
struct escm_lang * parse_lang(const char *name);
#endif /* ENABLE_POLYGLOT */

/* proc_file_name(lang, name, outp) - process a file. The inpuf file
 * is specifeid by its name.
 */
static void
proc_file_name(struct escm_lang *lang, const char *file, FILE *outp)
{
  FILE *inp;

  inp = fopen(file, "r");
  if (inp == NULL) XERROR("can't open - %s", file);

  escm_file = file;
  escm_assign(lang, "escm_input_file", file, outp);

  escm_preproc(lang, inp, outp);
  fclose(inp);
}
/* main function
 */
int
main(int argc, char **argv)
{
  int i, c;
  FILE *inp;
  FILE *outp = stdout;
  int header_flag = TRUE;
  int process_flag = TRUE;
  char *output_file = NULL;
#ifdef ENABLE_POLYGLOT
  char *lang_name = NULL;
#endif /* ENABLE_POLYGLOT */
  char *interp = NULL;
  char **expr = NULL;
  int n_expr = 0;
  char *gateway_interface = NULL;
  char *path_translated = NULL;

#define OPTSTR "EHce:i:l:o:"
#if defined(HAVE_GETOPT_LONG)
  int long_idx;
  int help_flag = FALSE;
  const struct option long_opt[] = {
    { "no-eval", 0, NULL, 'E' },
    { "no-header", 0, NULL, 'H', },
    { "eval", 1, NULL, 'e', },
    { "interp", 1, NULL, 'i', },
    { "language", 1, NULL, 'l', },
    { "help", 0, &help_flag, TRUE, },
    { "version", 0, &help_flag, FALSE, },
    { NULL, 0, NULL, 0, }
  };
#endif /* defined(HAVE_GETOPT_LONG) */
  struct escm_lang *lang;

  /* the default language (scheme perhaps) */
  lang = &deflang;

  /* Check the environment. */
  gateway_interface = getenv("GATEWAY_INTERFACE");
  if (gateway_interface) {
    path_translated = getenv("PATH_TRANSLATED");
  }

  /* Set the program name. */
  escm_prog = argv[0];

  /* redirect stderr to stdout if it is invoked as CGI. */
  if (gateway_interface) {
    escm_redirect(fileno(stderr), fileno(stdout));
  }

  /* expand meta-arguments if necessary */
  if (path_translated) {
    if (argc != 1) {
      XERROR("too many arguments");
    }
    inp = parse_shebang(argv[0], path_translated, &argc, &argv);
  } else if (argc == 3 && argv[1][0] == '\\' && argv[1][1] == '\0') {
    inp = parse_shebang(argv[0], argv[2], &argc, &argv);
  } else {
    inp = NULL;
  }

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
"  -H, --no-header              print no content header even in a CGI\n"
"  -e, --eval=EXPR              evaluate an expression\n"
"  -i, --interp='PROG ARG ...'  invoke an interpreter as backend\n"
#ifdef ENABLE_POLYGLOT
"  -l, --language=LANG          choose the interpreter language\n"
#endif /* ENABLE_POLYGLOT */
"      --help                   print this message and exit\n"
"      --version                print version information and exit\n",
	       escm_prog);
	printf("\nReport bugs to <%s>\n", PACKAGE_BUGREPORT);
      } else {
	interp = getenv("ESCM_BACKEND");
	if (!interp) interp = ESCM_BACKEND;
	printf("%s - developers' version of escm\n", PACKAGE_STRING);
	printf("The default interpreter is '%s'\n", interp);
      }
      exit(EXIT_SUCCESS);
      /* not reached */
#endif /* defined(HAVE_GETOPT_LONG) */
    case 'E':
      process_flag = FALSE;
      break;
    case 'H':
      header_flag = FALSE;
      break;
    case 'c':
      header_flag = FALSE;
      if (!gateway_interface) escm_redirect(fileno(stderr), fileno(stdout));
      break;
    case 'e':
      n_expr++;
      expr = XREALLOC(char *, expr, n_expr);
      expr[n_expr - 1] = optarg;
      break;
    case 'i':
      interp = optarg;
      break;
#ifdef ENABLE_POLYGLOT
    case 'l':
      lang_name = optarg;
      break;
#endif /* ENABLE_POLYGLOT */
    case 'o':
      output_file = optarg;
      break;
    default:
#if defined(HAVE_GETOPT_LONG)
      printf("Try `%s --help' for more information.\n", escm_prog);
#else /* !defined(HAVE_GETOPT_LONG) */
# ifdef ENABLE_POLYGLOT
      fprintf(stderr, "Usage: %s [-EHc] [-e EXPR] [-i \"PROG ARG ...\"]\n       [-l LANG] [-o OUTPUT] FILE ...\n", escm_prog);
# else
      fprintf(stderr, "Usage: %s [-EHc] [-e EXPR] [-i \"PROG ARG ...\"]\n       [-o OUTPUT] FILE ...\n", escm_prog);
# endif /* ENABLE_POLYGLOT */
      fprintf(stderr, "%s - experimental version of escm\n", PACKAGE_STRING);
#endif /* defined(HAVE_GETOPT_LONG) */
      exit(EXIT_FAILURE);
      /* not reached */
    }
  }

  /* write out a content header. */
  if (header_flag && gateway_interface) {
    if (process_flag) cgi_html_header(outp);
    else cgi_text_header(outp);
  }

  /* specify the output file if necessary. */
  if (output_file) {
    if (freopen(output_file, "w", stdout) == NULL)
      XERROR("can't open - %s", output_file);
  }

#ifdef ENABLE_POLYGLOT
  /* select the language if necessary. */
  if (lang_name) {
    lang = parse_lang(lang_name);
  } else {
    lang_name = getenv("ESCM_DEFAULT");
    if (lang_name) lang = parse_lang(lang_name);
    if (!interp) interp = getenv("ESCM_BACKEND");
  }
  if (lang == NULL) XERROR("invalid language configuration");
#else /* not ENABLE_POLYGLOT */
  if (!interp) interp = getenv("ESCM_BACKEND");
#endif /* ENABLE_POLYGLOT */

  /* invoke the interpreter. */
  if (process_flag) {
    if (interp) outp = escm_popen(parse_cmdline(interp));
    else outp = escm_popen(lang->backend);
    /* outp will never be NULL. See fork.c */
  }

  /* initialization */
  escm_init(lang, outp);
  escm_bind(lang, "escm_version", PACKAGE " " VERSION, outp);
  if (path_translated) {
    escm_bind(lang, "escm_input_file", path_translated, outp);
  } else if (inp == NULL) {
    escm_bind(lang, "escm_input_file", NULL, outp);
  } else {
    escm_bind(lang, "escm_input_file", argv[argc - 1], outp);
  }
  escm_bind(lang, "escm_output_file", output_file, outp);
  if (process_flag) {
    escm_bind(lang, "escm_interpreter",
	      interp ? interp : ESCM_BACKEND, outp);
    if (gateway_interface) {
      const char *method;
      method = getenv("REQUEST_METHOD");
      if (method[0] == 'P') escm_bind_query_string(lang, outp);
      else escm_bind(lang, "escm_query_string", getenv("QUERY_STRING"), outp);
    } else {
      escm_bind(lang, "escm_query_string", NULL, outp);
    }
  } else {
    escm_bind(lang, "escm_interpreter", NULL, outp);
  }

  /* evaluate the expressions specified in options */
  for (i = 0; i < n_expr; i++) {
    fputs(expr[i], outp);
    fputc('\n', outp);
  }

  /* process files */
  if (path_translated) {
    escm_preproc(lang, inp, outp);
  } else if (!inp) { /* filter */
    if (argc == optind) {
      escm_file = "stdin";
      escm_preproc(lang, stdin, outp);
    } else {
      for (i = optind; i < argc; i++)
	proc_file_name(lang, argv[i], outp);
    }
  } else { /* wrapper of an interpreter */
    if (argc > optind + 1) XERROR("too many arguments");
    escm_preproc(lang, inp, outp);
  }

  /* finalization */
  escm_finish(lang, outp);

  /* close the pipe. */
  if (process_flag && escm_pclose(outp) != 0)
    XERROR("the backend exited unsuccessfully");
  return 0;
}
/* end of filter.c */
