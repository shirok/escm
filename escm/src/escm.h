/************************************************
 * escm.h - preprocess XML with scheme or another
 * interpreter language.
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 ***********************************************/
#ifndef ESCM_H
# define ESCM_H 1

#include <stdio.h>

/* Will be moved elsewhere. */
#define gettext(text) text

#ifndef FALSE
# define FALSE 0
#endif /* FALSE */
#ifndef TRUE
# define TRUE !FALSE
#endif /* TRUE */

/* Used to make error messages. */
extern const char *escm_prog;
extern const char *escm_file;
extern int escm_lineno;

/* Used here and there. */
extern const char *escm_cgi;

/* language information structure */
struct escm_lang {
  char *name;            /* the name space with the subname */
  char *literal_prefix;
  char *literal_suffix;
  char *display_prefix;
  char *display_suffix;
  char *bind_prefix;
  char *bind_infix;
  char *bind_suffix;
  char *assign_prefix;
  char *assign_infix;
  char *assign_suffix;
  int use_hyphen;
  char *nil;
  char *init;            /* initialization code */
  char *finish;          /* finalization code */
};

extern struct escm_lang deflang;

/* escm.c  */
/* escm_init(&lang, outp) - initialize the backend interpreter. */
void escm_init(const struct escm_lang *lang, FILE *outp);
/* escm_finish(&lang, outp) - finalize the backend interpreter. */
void escm_finish(const struct escm_lang *lang, FILE *outp);
/* escm_preproc(&lang, inp, outp) - the preprocessor. */
void escm_preproc(const struct escm_lang *lang, FILE *inp, FILE *outp);
/* escm_bind(lang, var, val, outp) - bind var to val. */
void escm_bind(const struct escm_lang *lang, const char *var, const char *val, FILE *outp);
/* escm_bind_query_string(lang, outp) - bind QUERY_STRING when the method is POST. */
void escm_bind_query_string(const struct escm_lang *lang, FILE *outp);
/* escm_assign(lang, var, val, outp) - assign var to val. */
void escm_assign(const struct escm_lang *lang, const char *var, const char *val, FILE *outp);
/* escm_literal(lang, str, outp) - put a string in lang. */
void escm_literal(const struct escm_lang *lang, const char *str, FILE *outp);

/* cgi.c */
/* escm_header(lang, outp) - send an HTML content-type header. */
void escm_header(const struct escm_lang *lang, FILE *outp);

/* escm_is_cgi() - return TRUE if invoked in a CGI script, FALSE otherwise. */
int escm_is_cgi(void);
/* escm_error(fmt, ...) - print a warning message and exit the program. */
void escm_error(const char *fmt, ...);

/* fork.c */
FILE *escm_popen(const char *prog);
int escm_pclose(FILE *fp);
#endif /* ESCM_H */
