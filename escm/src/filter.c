/*******************************************************
 * filter.c - main function for the filter tool
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
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

#include "meta_arg.h"
#include "escm.h"
#include "misc.h"

#ifdef ENABLE_CGI
/* Will be moved elesewhere. */
static char * env_to_bind[] = {
  /* "GATEWAY_INTERFACE", */
  "HTTP_ACCEPT_LANGUAGE",
  "HTTP_ACCEPT_CHARSET",
  "HTTP_COOKIE",
  "HTTP_HOST",
  "HTTP_REFERER",
  "HTTP_USER_AGENT",
  /* "QUERY_STRING", */
  "REMOTE_ADDR",
  /* "REQUEST_METHOD", */
};
#endif /* ENABLE_CGI */

#define SizeOfArray(arr) (sizeof(arr) / (sizeof(arr[0])))

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
  if (inp == NULL) escm_error(_("can't open - %s"), file);

  escm_file = file;
  escm_assign(lang, "*escm-input-file*", file, outp);

  meta_skip_shebang(inp);
  escm_preproc(lang, inp, outp);
  fclose(inp);
}
/* main function
 */
int
main(int argc, char **argv)
{
  int i, c, ret;
  FILE *outp = stdout;
  int header_flag = TRUE;
  int process_flag = TRUE;
  char *footer_file = NULL;
  char *output_file = NULL;
#ifdef ENABLE_POLYGLOT
  char *lang_name = NULL;
#endif /* ENABLE_POLYGLOT */
  char *interp = NULL;
  char **expr = NULL;
  int n_expr = 0;
#ifdef ENABLE_CGI
  const char *path_translated = NULL;
#endif /* ENABLE_CGI */
#if defined(HAVE_GETOPT_LONG)
  int long_idx;
  int help_flag = FALSE;
  const struct option long_opt[] = {
    { "no-eval", 0, NULL, 'E' },
    { "no-header", 0, NULL, 'H', },
    { "eval", 1, NULL, 'e', },
    { "footer", 1, NULL, 'f', },
    { "interp", 1, NULL, 'i', },
    { "language", 1, NULL, 'l', },
    { "output", 1, NULL, 'o', },
    { "help", 0, &help_flag, TRUE, },
    { "version", 0, &help_flag, FALSE, },
    { NULL, 0, NULL, 0, }
  };
#endif /* defined(HAVE_GETOPT_LONG) */
  struct escm_lang *lang;

  /* the default language (scheme perhaps) */
  lang = &deflang;

  /* Check the environment. */
#ifdef ENABLE_CGI
  escm_cgi = getenv("GATEWAY_INTERFACE");
#endif /* ENABLE_CGI */

  /* Set the program name. */
  escm_prog = argv[0];

#ifdef ENABLE_CGI
  /* redirect stderr to stdout if it is invoked as CGI. */
  if (escm_cgi) {
    escm_redirect(fileno(stderr), fileno(stdout));
    path_translated = getenv("PATH_TRANSLATED");
  }
#endif /* ENABLE_CGI */

  /* expand meta-arguments if necessary */
#ifdef ENABLE_CGI
  if (argc == 1 && path_translated) {
    ret = meta_args_replace(&argc, &argv, path_translated, 1);
  } else {
#endif /* ENABLE_CGI */
    ret = meta_args(&argc, &argv);
#ifdef ENABLE_CGI
  }
#endif /* ENABLE_CGI */

  /* process options */
  for (;;) {
    c = getopt_long(argc, argv, "EHe:f:i:l:o:", long_opt, &long_idx);
    if (c == -1) break;
    switch (c) {
#if defined(HAVE_GETOPT_LONG)
    case 0:
      if (help_flag) {
	printf(_(
"Usage: %s [OPTION] ... FILE ...\n"
"Preprocess embedded scheme code in documents.\n"
"\n"
"  -E, --no-eval                convert documents into code\n"
"  -H, --no-header              print no content header even in a CGI\n"
"  -e, --eval=EXPR              evaluate an expression\n"
"  -f, --footer=FILENAME        specify the footer file\n"
"  -i, --interp='PROG ARG ...'  invoke an interpreter as backend\n"
"  -l, --language=LANG          choose the interpreter language\n"
"  -o, --output=FILENAME        specify the output file\n"
"      --help                   print this message and exit\n"
"      --version                print version information and exit\n"),
	       escm_prog);
#ifndef ENABLE_CGI
	printf(_("Option %s is discarded.\n"), "-H");
#endif /* ENABLE_CGI */
#ifndef ENABLE_POLYGLOT
	printf(_("Option %s is discarded.\n"), "-l");
#endif /* ENABLE_POLYGLOT */
	printf(_("\nReport bugs to <%s>\n"), PACKAGE_BUGREPORT);
      } else {
	interp = getenv("ESCM_BACKEND");
	if (!interp) interp = ESCM_BACKEND;
	printf(_("%s - experimental version of escm\n"), PACKAGE_STRING);
	printf(_("The default interpreter is '%s'\n"), interp);
      }
      exit(EXIT_SUCCESS);
      /* not reached */
#endif /* defined(HAVE_GETOPT_LONG) */
    case 'E':
      process_flag = FALSE;
      break;
#ifdef ENABLE_CGI
    case 'H':
      header_flag = FALSE;
      break;
#endif /* ENABLE_CGI */
    case 'e':
      n_expr++;
      expr = XREALLOC(char *, expr, n_expr);
      expr[n_expr - 1] = optarg;
      break;
    case 'f':
      footer_file = optarg;
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
      printf(_("Try `%s --help' for more information.\n"), escm_prog);
#else /* !defined(HAVE_GETOPT_LONG) */
      fprintf(stderr, "Usage: %s [-EH] [-e EXPR][-f FOOTER] [-i \"PROG ARG ...\"]\n       [-l LANG] [-o OUTPUT] FILE ...\n", escm_prog);
      fprintf(stderr, "%s - experimental version of escm\n", PACKAGE_STRING);
#endif /* defined(HAVE_GETOPT_LONG) */
      exit(EXIT_FAILURE);
      /* not reached */
    }
  }

#ifdef ENABLE_CGI
  /* write out a content header. */
  if (header_flag && escm_cgi) {
    if (process_flag) escm_html_header(lang, outp);
    else escm_text_header(lang, outp);
  }
#endif /* ENABLE_CGI */

  /* specify the output file if necessary. */
  if (output_file) {
    if (freopen(output_file, "w", stdout) == NULL)
      escm_error(_("can't open - %s"), output_file);
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
#else /* not ENABLE_POLYGLOT */
  if (!interp) interp = getenv("ESCM_BACKEND");
#endif /* ENABLE_POLYGLOT */

  /* invoke the interpreter. */
  if (process_flag) {
    if (interp) outp = escm_popen(tokenize_cmd(interp));
    else outp = escm_popen(lang->backend);
    /* outp will never be NULL. See fork.c */
  }

  /* initialization */
  escm_init(lang, outp);
  escm_bind(lang, "*escm-version*", PACKAGE " " VERSION, outp);
  escm_bind(lang, "*escm-input-file*", NULL, outp);
  escm_bind(lang, "*escm-output-file*", output_file, outp);
  if (process_flag) {
    escm_bind(lang, "*escm-interpreter*",
	      interp ? interp : ESCM_BACKEND, outp);
#ifdef ENABLE_CGI
    escm_bind(lang, "GATEWAY_INTERFACE", escm_cgi, outp);
    /* set useful global variables if the language is scheme. */
    if (escm_cgi) {
      const char *method;
      int i;
      method = getenv("REQUEST_METHOD");
      escm_bind(lang, "REQUEST_METHOD", method, outp);
      if (method[0] == 'P') escm_bind_query_string(lang, outp);
      else escm_bind(lang, "QUERY_STRING", getenv("QUERY_STRING"), outp);
      for (i = 0; i < SizeOfArray(env_to_bind); i++) {
	escm_bind(lang, env_to_bind[i], getenv(env_to_bind[i]), outp);
      }
    }
#endif /* ENABLE_CGI */
  }

  /* evaluate the expressions specified in options */
  for (i = 0; i < n_expr; i++) {
    fputs(expr[i], outp);
    fputc('\n', outp);
  }

  /* content header if -E */
  if (header_flag && !process_flag)
    escm_literal(lang, "Content-type: text/html\\r\\n\\r\\n", outp);

  /* process files */
  if (argc == optind) {
    escm_file = "stdin";
    escm_lineno = 1;
    escm_preproc(lang, stdin, outp);
  } else {
    for (i = optind; i < argc; i++) {
      proc_file_name(lang, argv[i], outp);
    }
  }
  if (footer_file)
    proc_file_name(lang, footer_file, outp);

  /* finalization */
  escm_finish(lang, outp);

  /* close the pipe. */
  if (process_flag && escm_pclose(outp) != 0)
    escm_error("the backend exited unsuccessfully");
  return 0;
}
/* end of filter.c */
