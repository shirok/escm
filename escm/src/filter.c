/*******************************************************
 * filter.c - main function for the filter tool
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
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include "meta_arg.h"
#include "escm.h"

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
struct escm_lang * parse_lang(const char *name, const char **interp);
#endif /* ENABLE_POLYGLOT */

/* usage() - print a short help message.
 */
static void
usage(void)
{
  printf(gettext("Usage: %s [OPTION] ... FILE ...
Process embedded scheme code in documents.
  -E                  only preprocess files
  -H                  print no content header even in a CGI script
  -e EXPR             evaluate an expression
  -f FILENAME         specify the footer file
  -i 'PROG ARG ... '  explicitly specify the interpreter
  -l LANG             specify another interpreter language than scheme
  -o FILENAME         specify the output file
  -h                  print this message
  -v                  print version information"), escm_prog);
#ifndef ENABLE_CGI
  printf(gettext("\nOption %s is discarded."), "-H");
#endif /* ENABLE_CGI */
#ifndef ENABLE_POLYGLOT
  printf(gettext("\nOption %s is discarded."), "-l");
#endif /* ENABLE_POLYGLOT */
  printf(gettext("\n\nReport bugs to <%s>\n"), PACKAGE_BUGREPORT);
}

/* version() - print version information.
 */
static void
version(void)
{
  fputs(PACKAGE_STRING " - experimental version of escm\n", stdout);
  fputs("The default interpreter is '" ESCM_BACKEND "'\n", stdout);
}

/* proc_file_name(lang, name, outp) - process a file. The inpuf file
 * is specifeid by its name.
 */
static void
proc_file_name(struct escm_lang *lang, const char *file, FILE *outp)
{
  FILE *inp;

  inp = fopen(file, "r");
  if (inp == NULL) escm_error(gettext("can't open - %s"), file);

  escm_file = file;
  escm_assign(lang, "*escm-input-file*", file, outp);

  meta_skip_shebang(inp);
  escm_preproc(lang, inp, outp);
  fclose(inp);
}

/* struct opt_data - structure to store options.
 */
struct opt_data {
  const char *interpreter;
  const char *outfile;
  const char *langname;
  const char *footer;
  char **expr;
  int n_expr;
  int process;
  int header;
};

/* parse_opts(argc, argv, &opts) - process options
 */
static void
parse_opts(int argc, char **argv, struct opt_data *opt)
{
  int c;

  for (;;) {
    c = getopt(argc, argv, "EHe:f:i:l:o:hv");
    if (c == -1) break;
    switch (c) {
    case 'E':
      opt->process = FALSE;
      break;
#ifdef ENABLE_CGI
    case 'H':
      opt->header = FALSE;
      break;
#endif /* ENABLE_CGI */
    case 'e':
      opt->n_expr++;
      opt->expr = (char**)realloc(opt->expr, sizeof(char*) * opt->n_expr);
      if (opt->expr == NULL) escm_error(NULL);
      opt->expr[opt->n_expr - 1] = optarg;
      break;
    case 'f':
      opt->footer = optarg;
      break;
    case 'i':
      opt->interpreter = optarg;
      break;
#ifdef ENABLE_POLYGLOT
    case 'l':
      opt->langname = optarg;
      break;
#endif /* ENABLE_POLYGLOT */
    case 'o':
      opt->outfile = optarg;
      break;
    case 'v':
      version();
      exit(EXIT_SUCCESS);
      /* not reached */
    case 'h':
      usage();
      exit(EXIT_SUCCESS);
      /* not reached */
    default:
      usage();
      exit(EXIT_FAILURE);
      /* not reached */
    }
  }
}

/* main function
 */
int
main(int argc, char **argv)
{
  int i, ret;
  FILE *outp = stdout;
  struct opt_data  opts = {
    NULL, /* interpreter */
    NULL, /* outfile */
    NULL, /* langname */
    NULL, /* footer */
    NULL, /* expr */
    0, /* n_expr */
    TRUE, /* process */
    TRUE, /* header */
  };
  struct escm_lang *lang;
#ifdef ENABLE_CGI
  const char *path_translated = NULL;
#endif /* ENABLE_CGI */
  lang = &deflang;

  /* Check the environment. */
#ifdef ENABLE_CGI
  escm_cgi = getenv("GATEWAY_INTERFACE");
#endif /* ENABLE_CGI */
  opts.interpreter = getenv("ESCM_BACKEND");
#ifdef ENABLE_POLYGLOT
  opts.langname = getenv("ESCM_DEFAULT");
#endif /* ENABLE_POLYGLOT */

  /* for error messages */
  escm_prog = meta_progname(argv[0]);

#ifdef ENABLE_CGI
  /* redirect stderr to stdout if it is invoked as CGI. */
  if (escm_cgi) {
    if (dup2(fileno(stdout), fileno(stderr)) < 0) escm_error(NULL);
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
  parse_opts(argc, argv, &opts);

#ifdef ENABLE_CGI
  /* write out a content header. */
  if (opts.header && escm_cgi) {
    if (opts.process) escm_html_header(lang, outp);
    else escm_text_header(lang, outp);
  }
#endif /* ENABLE_CGI */

  /* specify the output file if necessary. */
  if (opts.outfile) {
    if (freopen(opts.outfile, "w", stdout) == NULL)
      escm_error(gettext("can't open - %s"), opts.outfile);
  }

#ifdef ENABLE_POLYGLOT
  /* select the language if necessary. */
  if (opts.langname) lang = parse_lang(opts.langname, &(opts.interpreter));
#endif /* ENABLE_POLYGLOT */

  /* invoke the interpreter. */
  if (opts.process) {
    outp = escm_popen(opts.interpreter);
    /* outp will never be NULL. See fork.c */
  }

  /* initialization */
  escm_init(lang, outp);
  escm_bind(lang, "*escm-version*", PACKAGE " " VERSION, outp);
  escm_bind(lang, "*escm-input-file*", NULL, outp);
  escm_bind(lang, "*escm-output-file*", opts.outfile, outp);
  if (opts.process) {
    escm_bind(lang, "*escm-interpreter*",
	      opts.interpreter ? opts.interpreter : ESCM_BACKEND, outp);
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
  for (i = 0; i < opts.n_expr; i++) {
    fputs(opts.expr[i], outp);
    fputc('\n', outp);
  }

  /* content header if -E */
  if (opts.header && !opts.process)
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
  if (opts.footer)
    proc_file_name(lang, opts.footer, outp);

  /* finalization */
  escm_finish(lang, outp);

  /* close the pipe. */
  if (opts.process && escm_pclose(outp) != 0)
    escm_error(NULL);

  return 0;
}
/* end of filter.c */
