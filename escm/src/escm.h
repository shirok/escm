/************************************************
 * escm.h - preprocess XML with scheme or another
 * interpreter language.
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 ***********************************************/
#ifndef ESCM_H
# define ESCM_H 1

#include <stdio.h>

#ifndef FALSE
# define FALSE 0
#endif /* FALSE */
#ifndef TRUE
# define TRUE !FALSE
#endif /* TRUE */

/* language information structure */
struct escm_lang {
  const char *name;            /* the name space with the subname */
  const char *literal_prefix;
  const char *literal_suffix;
  const char *display_prefix;
  const char *display_suffix;
  const char *init;            /* initialization code */
  const char *finish;          /* finalization code */
};

/* escm.c  */
/* escm_init(&lang, outp) - initialize the backend interpreter. */
void escm_init(struct escm_lang *lang, FILE *outp);
/* escm_finish(&lang, outp) - finalize the backend interpreter. */
void escm_finish(struct escm_lang *lang, FILE *outp);
/* escm_preproc(&lang, inp, outp) - the preprocessor. */
int escm_preproc(struct escm_lang *lang, FILE *inp, FILE *outp);
/* escm_put_string(str, outp) - escape str and put it.*/
void escm_put_string(const char *str, FILE *outp);

/* cgi.c */

/* escm_is_cgi() - return TRUE if invoked in a CGI script, FALSE otherwise. */
int escm_is_cgi(void);
/* escm_stderr2stdout() - redirect stderr to stdout. */
int escm_stderr2stdout(void);
/* escm_html_header() - send an HTML content-type header. */
void escm_html_header(void);
/* escm_html_header() - send a plain text content-type header. */
void escm_plain_header(void);
/* escm_warning(prog, msg) - print a warning message.*/
void escm_warning(const char *prog, const char *msg);
/* escm_error(msg) - print a warning message and exit the program. */
void escm_error(const char *prog, const char *msg);
#endif /* ESCM_H */
