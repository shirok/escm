/* escm.h - filter for XML process instructions
 * $Id$
 */
#ifndef ESCM_H
#define ESCM_H

#include "../parse/parse.h"
#include "../cgi.h"

/* Boolean values */
#ifndef FALSE
#define FALSE 0
#endif /* FALSE */
#ifndef TRUE
#define TRUE (!FALSE)
#endif /* TRUE */

/* Input form a file or stdin. */
#ifndef escm_inp_t
#define escm_inp_t FILE
#define escm_getc(inp) getc(inp)
#define escm_ungetc(c, inp) ungetc(c, inp)
#define escm_inp_open(file) fopen(file, "r")
#define escm_inp_close(fp) fclose(fp)
#define ESCM_STDIN stdin
#endif /* escm_inp_t */

/* Output to the interpreter */
#ifndef escm_outp_t
#define escm_outp_t FILE
#define escm_putc(c, outp) putc(c, outp)
#define escm_puts(s, outp) fputs(s, outp)
#define ESCM_STDOUT stdout
#endif /* escm_outp_t */

#ifndef HAVE_ESCM_PUTS
#define HAVE_ESCM_PUTS 0
#endif /* HAVE_ESCM_PUTS */

#if !HAVE_ESCM_PUTS && !defined(escm_puts)
/* The last resort version of escm_puts(). Implemented in escm.c */
extern void escm_puts(char *s, escm_outp_t *outp);
#endif /* escm_puts */

/* Connection to the backend interpreter (fork.c) */
extern escm_outp_t * escm_connect(char *const argv[]);
extern int escm_disconnect(escm_outp_t *pipe);

/* conversion of variable names */
enum escm_varname {
  ESCM_VN_NOCONV,
  ESCM_VN_HYPHEN,
  ESCM_VN_UPPER,
  ESCM_VN_LOWER,
  ESCM_VN_TITLE,
};
/* used in the preprocessor . */
enum escm_state {
  ESCM_LITERAL,
  ESCM_DISPLAY,
  ESCM_CODE,
};

/* Struct to store language data. */
struct escm_lang {
  const char *name;
  char * const * argv;
  const char * init;
  const char * finish;
  const char *true;
  const char *false;
  const char *display;
  enum escm_varname varname;
  const char *define_prefix;
  const char *define_infix;
  const char *define_suffix;
  const char *literal_prefix;
  const char *literal_suffix;
  const char *display_prefix;
  const char *display_suffix;
};

/* Struct to store style data */
struct escm_style {
  const int prefix_char;
  const int suffix_char;
  int (*change_state)(struct escm_lang *lang, escm_inp_t *in, escm_outp_t *out,
		     enum escm_state *state);
};
/* escm.c */
#ifdef ESCM_ENABLE_XMLPI
extern struct escm_style escm_xmlpi_style;
#endif /* ESCM_ENABLE_XMLPI */
#ifdef ESCM_ENABLE_SCRIPT
extern struct escm_style escm_script_style;
#endif /* ESCM_ENABLE_SCRIPT */
#ifdef ESCM_ENABLE_ARCHAIC
extern struct escm_style escm_archaic_style;
#endif /* ESCM_ENABLE_ARCHAIC */

extern void escm_define(struct escm_lang *lang, const char *varname, const char *expr,
			escm_outp_t *pout, int string_p);
extern void escm_preproc(struct escm_lang *lang, struct escm_style *style,
			 escm_inp_t *in, escm_outp_t *out, int c);

#ifndef ESCM_VARIABLE_PREFIX
#define ESCM_VARIABLE_PREFIX "escm_"
#endif /* ESCM_VARIABLE_PREFIX */
#define ESCM_PREFIX(str) (ESCM_VARIABLE_PREFIX str)

enum escm_flags {
  ESCM_NONE = 0x00,
  ESCM_CGI_FLAG = 0x01,
  ESCM_NOHEADER_FLAG = 0x02,
  ESCM_NOPROCESS_FLAG = 0x04,
};
#define ESCM_CGI_P(flags) ((flags) & ESCM_CGI_FLAG)
#define ESCM_NOHEADER_P(flags) ((flags) & ESCM_NOHEADER_FLAG)
#define ESCM_NOPROCESS_P(flags) ((flags) & ESCM_NOPROCESS_FLAG)

#define ESCM_PUTS_LITERAL(lang, string, pout) {\
    escm_puts((lang)->literal_prefix, pout);\
    escm_puts(string, pout);\
    escm_puts((lang)->literal_suffix, pout);\
  }
#define ESCM_DEFINE_STRING(lang, var, val, pout) \
   escm_define(lang, var, val, pout, TRUE)
#define ESCM_DEFINE(lang, var, val, pout) \
   escm_define(lang, var, val, pout, FALSE)
#define ESCM_DEFINE_BOOL(lang, var, val, pout) \
   escm_define(lang, var, val ? lang->true : lang->false, pout, FALSE)

#define ESCM_INIT(lang, pout) \
   if ((lang)->init) { escm_puts((lang)->init, pout); escm_putc('\n', pout); }
#define ESCM_FINISH(lang, pout) \
   if ((lang)->finish) { escm_puts((lang)->finish, pout); escm_putc('\n', pout); }

#endif /* ESCM_H */
/* end of escm.h */
