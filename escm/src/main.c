/* main.c - main function of `aescm'
 * $Id$
 *
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "config.h"
#include "cgi.h"
#include "escm/escm.h"
#include "copyright.h"

/* In this file is defined static struct escm_lang * const lang[]  */
#define IN_MAIN_C 1
#include "langconf.h"
#undef IN_MAIN_C

#define SizeOfArray(arr) (sizeof(arr) / (sizeof(arr[0])))

#ifdef ESCM_PLURISCRIPT
/* escm_find_lang(lang) - return the pointer to the language data.
 * or NULL.
 */
struct escm_lang *
escm_find_lang(const char * name)
{
  /* use the linear search method, for the number of supported languages
   * is so small.
   */
  int i;
  for (i = 0; i < SizeOfArray(lang); i++) {
    if (strcmp(name, lang[i]->name) == 0) return lang[i];
  }
  return NULL;
}
/* escm_list_langs() - print out a list of languages
 */
void
escm_list_langs(void)
{
  int i;
  for (i = 0; i < SizeOfArray(lang); i++) {
    printf("%s\n", lang[i]->name);
  }
}
#endif /* ESCM_PLURISCRIPT */


/* version_info() - print version information.
 */
static void
version_info(void)
{
  fputs(PACKAGE " - " VERSION "\n", stdout);
  fputs(COPYRIGHT, stdout);
  printf("  The default interpreter: %s\n", (ESCM_DEFAULT_LANGPTR)->argv[0]);
  exit(0);
}
/* usage() - print a short help message.
 */
static void
usage(void)
{
  fputs("Usage: " PACKAGE " " ESCM_USAGE, stdout);
  exit('?' != optopt);
}

struct escm_info {
  int argc;
  char **argv;
  char *outfile;
  struct escm_lang *lang;
  struct escm_style *style;
  char * const *backend;
  unsigned int cgi_p: 1;
  unsigned int no_header_p: 1;
  unsigned int no_process_p: 1;
  unsigned int no_skip_shebang_p: 1;
  int n_expr;
  char *expr[ESCM_N_EXPR];
};
static void
parse_options(struct escm_info *info)
{
  int optc;
  int dummy_argc;

  if (is_cgi()) info->cgi_p |= TRUE;

  opterr = 0;
  while ((optc = getopt(info->argc, info->argv, "!EHSalsvxL:e:i:o:")) != -1) {
    switch (optc) {
    case '!': /* #! */
      /* nop */
      break;
    case 'E': /* debugging */
      info->no_process_p |= TRUE;
      break;
    case 'H': /* no header even in a CGI script */
      info->no_header_p |= TRUE;
      break;
#ifdef ESCM_PLURISCRIPT
    case 'L': /* lang */
      info->lang = escm_find_lang(optarg);
      if (info->lang == NULL) cgi_error("Language <<%s>> not found", optarg);
      break;
#endif /* ESCM_PLURISCRIPT */
    case 'S': /* do not skip #! line */
      info->no_skip_shebang_p |= TRUE;
      break;
#ifdef ESCM_ENABLE_ARCHAIC
    case 'a': /* use the archaical style <? expr ... !> */
      info->style = &escm_archaic_style;
      break;
#endif /* ESCM_ENABLE_ARCHAIC */
    case 'e': /* emit an expression */
      if (info->n_expr >= ESCM_N_EXPR) cgi_error("too many -e");
      info->expr[info->n_expr++] = optarg;
      break;
    case 'i': /* specify the interpreter */
      info->backend = command_line(optarg, &dummy_argc);
      break;
#ifdef ESCM_PLURISCRIPT
    case 'l': /* list the supported langs */
      escm_list_langs();
      exit(0);
      /* not reached */
#endif /* ESCM_PLURISCRIPT */
    case 'o': /* specfy the output file */
      info->outfile = optarg;
      break;
#ifdef ESCM_ENABLE_SCRIPT
    case 's': /* use the script style <% expr ... %> */
      info->style = &escm_script_style;
      break;
#endif /* ESCM_ENABLE_SCRIPT */
    case 'v': /* show the version information */
      version_info();
      /* not reached */
#ifdef ESCM_ENABLE_XMLPI
    case 'x': /* use the XML style <?LANG expr ... ?> */
      info->style = &escm_xmlpi_style;
      break;
#endif /* ESCM_ENABLE_XMLPI */
    default: /* option error */
      usage();
      /* not reached */
    }
  }
}

/* skip_shebang(inp) - skip the shebang line and return the character
 * to be pushed back. It returns EOF if there is none or
 * it has really reached the EOF.
 */
static int
skip_shebang(escm_inp_t *in)
{
  int c;
  c = escm_getc(in);
  if (c == EOF) return EOF;
  else if (c != '#') return c;
  c = escm_getc(in);
  if (c == EOF) return '#';
  else if (c != '!') {
    escm_ungetc(c, in);
    return '#';
  } else {
    int nfield = 0;
    int in_field = FALSE;
    while ((c = escm_getc(in)) != '\n' && c != EOF) {
      if (isspace(c)) {
	in_field = FALSE;
      } else if (c == '\\' && !in_field && nfield == 1) {
	c = escm_getc(in);
	if (c == EOF) {
	  break;
	} else if (c == '\n') {
	  /* skip one more line */
	  while ((c = escm_getc(in)) != '\n' && c != EOF) {
	    /* nop */
	  }
	  break;
	} else {
	  in_field = TRUE;
	  nfield++;
	}
      } else {
	if (!in_field) {
	  in_field = TRUE;
	  nfield++;
	}
      }
    }
    return EOF;
  }
}
/* connect with server and intialize */
static escm_outp_t *
connect_and_initialize(struct escm_info *info)
{
  escm_outp_t *pout;
  int i;

  /* redirect stdout if necessary. */
  if (info->outfile && freopen(info->outfile, "w", stdout) == NULL)
    cgi_error("Can't open %s\n", info->outfile);

  /* use the default backend if not specified. */
  if (! info->backend) info->backend = (info->lang)->argv;

  /* connect with the server. */
  if (info->no_process_p) {
    pout = ESCM_STDOUT;
    if (info->cgi_p)
      escm_puts("Content-type: text/plain\r\n\r\n", pout);
  } else pout = escm_connect(info->backend);

  ESCM_INIT(info->lang, pout);
  if (info->cgi_p && !info->no_header_p) {
    ESCM_PUTS_LITERAL(info->lang, "Content-type: text/html\\r\\n\\r\\n", pout);
    escm_putc('\n', pout);
  }

  /* send configurations */
  ESCM_DEFINE_STRING(info->lang, ESCM_PREFIX("version"), VERSION, pout);
  ESCM_DEFINE_STRING(info->lang, ESCM_PREFIX("output_file"), info->outfile, pout);
  ESCM_DEFINE_STRING(info->lang, ESCM_PREFIX("interpreter"), info->backend[0], pout);
  ESCM_DEFINE_BOOL(info->lang, ESCM_PREFIX("cgi_script"), info->cgi_p, pout);

  /* send -e option strings */
  for (i = 0; i < info->n_expr; i++) {
    escm_puts(info->expr[i], pout);
    escm_putc('\n', pout);
  }
  return pout;
}

/* finalize and disconnect */
static int
finalize_and_disconnect(escm_outp_t *pout, struct escm_info *info)
{
  ESCM_FINISH(info->lang, pout);

  /* disconnect from the server */
  if (info->no_process_p) return 0;
  else return escm_disconnect(pout);
}

/* main loop for without a metaswitch */
static int
main_filter_loop(struct escm_info *info)
{
  escm_inp_t *fin;
  escm_outp_t *pout;
  int i;
  if (info->argc > 2 && info->argv[1][0] == '-' && info->argv[1][1] == '!') {
    info->argv = shebang(info->argv, &(info->argc));
  }
  parse_options(info);

  /* connect and initialize */
  pout = connect_and_initialize(info);

  /* process files or stdin */
  if (info->argc > optind) {
    for (i = optind; i < info->argc; i++) {
      fin = escm_inp_open(info->argv[i]);
      if (fin == NULL) cgi_error("Can't open %s\n", info->argv[i]);
      ESCM_DEFINE_STRING(info->lang, ESCM_PREFIX("input_file"), info->argv[i], pout);
      escm_preproc(info->lang, info->style, fin, pout,
		   info->no_skip_shebang_p ? EOF : skip_shebang(fin));
      escm_inp_close(fin);
    }
  } else {
    ESCM_DEFINE_BOOL(info->lang, ESCM_PREFIX("input_file"), FALSE, pout);
    escm_preproc(info->lang, info->style, ESCM_STDIN, pout,
		 info->no_skip_shebang_p ? EOF : skip_shebang(ESCM_STDIN));
  }
  return finalize_and_disconnect(pout, info);
}

/* main function */
int
main(int argc, char **argv)
{
  struct escm_info info = {
    argc,
    argv,
    NULL, /* outfile */
    ESCM_DEFAULT_LANGPTR, /* lang */
    ESCM_DEFAULT_STYLEPTR, /* style */
    NULL, /* backend */
    FALSE, /* cgi_p */
    FALSE, /* no_header_p */
    FALSE, /* no_process_p */
    FALSE, /* no_skip_shebang_p */
    0, /* n_expr */
    /* expr */
  };

  if (argc > 2 && argv[1][0] == '\\' && argv[1][1] == '\0') {
    /* FIXME: this implementation is too simplistic. */
    escm_inp_t *fin;
    escm_outp_t *pout;
    char buf[512]; /* big enough ? */

    /* Open argv[2]. The other arguments, if any, are discarded. */
    fin = escm_inp_open(argv[2]);
    if (fin == NULL) cgi_error("Can't open %s\n", argv[2]);

    /* skip the first line */
    if (! fgets(buf, SizeOfArray(buf), fin)) return 0;

    /* Get a line, add a dummy argv[0] to it, and parse it as if it were a
     * command line.
     */
    buf[0] = '@', buf[1] = ' ';
    if (! fgets(buf + 2, SizeOfArray(buf) - 2, fin)) return 0;
    info.argv = command_line(buf, &(info.argc));

    /* Call getopt(). Non-optional arguments, if any, are discarded. */
    parse_options(&info);

    /* Process the rest of the file. */

    /* connect and initialize */
    pout = connect_and_initialize(&info);

    ESCM_DEFINE_STRING(info.lang, ESCM_PREFIX("input_file"), argv[2], pout);
    escm_preproc(info.lang, info.style, fin, pout, EOF);
    escm_inp_close(fin);
    return finalize_and_disconnect(pout, &info);
  } else {
    return main_filter_loop(&info);
  }
}

/* end of main.c */
