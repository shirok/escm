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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include "meta_arg.h"
#include "escm.h"
#include "escm_scm.h"

#ifdef ESCM_LANG_DIR
/* defined in lang.c */
struct escm_lang * parse_lang(const char *name);
#endif /* ESCM_LANG_DIR */

/* usage() - print a short help message.
 */
static void
usage(void)
{
  printf("Usage: " PACKAGE " [OPTION] ... FILE ...
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

Report bugs to <" PACKAGE_BUGREPORT ">.\n");
}

/* version() - print version information.
 */
static void
version(void)
{
  printf(PACKAGE_STRING " - experimental version of escm\n");
  printf("The default interpreter is '%s'\n", scm_interp);
}

/* proc_file_fp(lang, inp, outp) - process a file. The inpuf file
 * is specifeid by the pointer to the FILE structure.
 */
static void
proc_file_fp(struct escm_lang *lang, FILE *inp, FILE *outp)
{
  int ret;
  ret = escm_preproc(lang, inp, outp);
  if (!ret) escm_error(PACKAGE, "Syntax error while reading from a stream.");
}
/* proc_file_name(lang, name, outp) - process a file. The inpuf file
 * is specifeid by its name.
 */
static void
proc_file_name(struct escm_lang *lang, const char *file, FILE *outp)
{
  int ret;
  FILE *inp;

  inp = fopen(file, "r");
  if (inp == NULL) escm_error(PACKAGE, NULL);

  meta_skip_shebang(inp);
  ret = escm_preproc(lang, inp, outp);
  if (!ret) escm_error(PACKAGE, "Syntax error while reading from a file");
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
    c = getopt(argc, argv, "EH:e:f:i:l:o:hv");
#else
    c = getopt(argc, argv, "EH:e:f:i:o:hv");
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
      if (opt->expr == NULL) escm_error(PACKAGE, NULL);
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
    NULL, /* *footer */
    NULL, /* expr */
    0, /* n_expr */
    TRUE, /* process */
    TRUE, /* header */
  };
  struct escm_lang *lang;
  lang = &lang_scm;

  /* redirect stderr to stdout. */
  if (escm_is_cgi()) escm_stderr2stdout();

  /* expand meta-arguments if necessary */
  ret = meta_args(&argc, &argv);

  /* process options */
  parse_opts(argc, argv, &opts);

  /* specify the output file if necessary. */
  if (opts.outfile) {
    if (freopen(opts.outfile, "w", stdout) == NULL)
      escm_error(PACKAGE, "Can't open output file.");
  }

#ifdef ESCM_LANG_DIR
  /* select the language if necessary. */
  if (opts.langname) lang = parse_lang(opts.langname, &(opts.interpreter));
#endif /* ESCM_LANG_DIR */

  /* write out a content header. */
  if (opts.header && escm_is_cgi()) {
    escm_html_header();
  }

  /* invoke the interpreter. */
  if (!opts.interpreter) opts.interpreter = scm_interp;
  if (opts.process) {
    outp = popen(opts.interpreter, "w");
    if (outp == NULL)
      escm_error(PACKAGE, "Can't invoke the interpreter.");
  }

  /* initialization */
  escm_init(lang, outp);

  /* set useful global variables if the language is scheme. */
  if (!opts.langname) {
    define_string("*escm-version*", PACKAGE " " VERSION, outp);
    define_string("*escm-interpreter*", opts.interpreter, outp);
    define_string("*escm-output-file*", opts.outfile, outp);
    /* input files */
    fprintf(outp, "(define *escm-input-file* '(");
    for (i = optind; i < argc; i++) {
      fputc(' ', outp);
      escm_put_string(argv[i], outp);
    }
    if (opts.footer) {
      fputc(' ', outp);
      escm_put_string(opts.footer, outp);
    }
    fprintf(outp, "))\n");
  }

  /* evaluate the expressions specified in options */
  for (i = 0; i < opts.n_expr; i++) {
    fprintf(outp, opts.expr[i]);
    fputc('\n', outp);
  }

  /* process files */
  if (argc == optind) proc_file_fp(lang, stdin, outp);
  else {
    for (i = optind; i < argc; i++) {
      proc_file_name(lang, argv[i], outp);
    }
  }
  if (opts.footer) proc_file_name(lang, opts.footer, outp);

  /* finalization */
  escm_finish(lang, outp);

  /* close the pipe. */
  if (opts.process && pclose(outp) == -1)
    escm_error(PACKAGE, NULL);

  return 0;
}
/* end of filter.c */
