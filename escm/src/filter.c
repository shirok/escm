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

#define SizeOfArray(arr) (sizeof(arr) / (sizeof(arr[0])))

#ifdef ESCM_LANG_DIR
/* defined in lang.c */
struct escm_lang * parse_lang(const char *name, const char **interp);
#endif /* ESCM_LANG_DIR */

/* usage() - print a short help message.
 */
static void
usage(void)
{
  printf(
"Usage: " PACKAGE " [OPTION] ... FILE ...
Process embedded scheme code in documents.
  -E                  only preprocess files
  -H                  print no content header even in a CGI script
  -e EXPR             evaluate an expression
  -f FILENAME         specify the footer file
  -i 'PROG ARG ... '  explicitly specify the interpreter
"
#ifdef ESCM_LANG_DIR
"  -l LANG             specify another interpreter language than scheme
"
#endif /* ESCM_LANG_DIR */
"  -o FILENAME         specify the output file
  -h                  print this message
  -v                  print version information
"
#ifdef ENABLE_HANDLER
"You can also use this tool as a handler CGI program.
"
#endif /* ENABLE_HANDLER */
"Report bugs to <" PACKAGE_BUGREPORT ">.\n");
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
#ifdef ESCM_LANG_DIR
    c = getopt(argc, argv, "EHe:f:i:l:o:hv");
#else
    c = getopt(argc, argv, "EHe:f:i:o:hv");
#endif /* ESCM_LANG_DIR */
    if (c == -1) break;
    switch (c) {
    case 'E':
      opt->process = FALSE;
      break;
    case 'H':
      opt->header = FALSE;
      break;
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
#ifdef ESCM_LANG_DIR
    case 'l':
      opt->langname = optarg;
      break;
#endif /* ESCM_LANG_DIR */
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
#ifdef ENABLE_HANDLER
  const char *path_translated = NULL;
#endif /* ENABLE_HANDLER */
  lang = &deflang;

  /* Check the environment. */
  escm_cgi = getenv("GATEWAY_INTERFACE");
  opts.interpreter = getenv("ESCM_BACKEND");
  opts.langname = getenv("ESCM_DEFAULT");

  /* for error messages */
  escm_prog = meta_progname(argv[0]);

  /* redirect stderr to stdout if it is invoked as CGI. */
  if (escm_cgi) {
    if (dup2(fileno(stdout), fileno(stderr)) < 0) escm_error(NULL);
#ifdef ENABLE_HANDLER
    path_translated = getenv("PATH_TRANSLATED");
#endif /* ENABLE_HANDLER */
  }

  /* expand meta-arguments if necessary */
#ifdef ENABLE_HANDLER
  if (argc == 1 && path_translated) {
    ret = meta_args_replace(&argc, &argv, path_translated, 1);
  } else {
#endif /* ENABLE_HANDLER */
    ret = meta_args(&argc, &argv);
#ifdef ENABLE_HANDLER
  }
#endif /* ENABLE_HANDLER */

  /* process options */
  parse_opts(argc, argv, &opts);

  /* write out a content header. */
  if (opts.header && escm_cgi) escm_header(lang, outp);

  /* specify the output file if necessary. */
  if (opts.outfile) {
    if (freopen(opts.outfile, "w", stdout) == NULL)
      escm_error(gettext("can't open - %s"), opts.outfile);
  }

#ifdef ESCM_LANG_DIR
  /* select the language if necessary. */
  if (opts.langname) lang = parse_lang(opts.langname, &(opts.interpreter));
#endif /* ESCM_LANG_DIR */

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
