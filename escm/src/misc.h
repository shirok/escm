/* misc.h - non public header for escm
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifndef MISC_H
#define MISC_H 1

#define FALSE 0
#define TRUE !FALSE

#if defined(ENABLE_NLS)
#  include <libintl.h>
#  define _(text) gettext(text)
#else /* !defined(ENABLE_NLS) */
#  define _(text) text
#endif /* defined(ENABLE_NLS) */

/* Used to make error messages. */
extern const char *escm_prog;
extern const char *escm_file;
extern int escm_lineno;
extern struct escm_lang deflang;

#ifdef ENABLE_CGI
extern const char *escm_cgi;
#endif /* ENABLE_CGI */

/* escm.c */
void escm_bind(const struct escm_lang *lang, const char *var, const char *val, FILE *outp);
#ifdef ENABLE_CGI
void escm_bind_query_string(const struct escm_lang *lang, FILE *outp);
#endif /* ENABLE_CGI */
void escm_assign(const struct escm_lang *lang, const char *var, const char *val, FILE *outp);

/* misc.c */
#ifdef ENABLE_CGI
void escm_html_header(const struct escm_lang *lang, FILE *outp);
void escm_text_header(const struct escm_lang *lang, FILE *outp);
#endif /* ENABLE_CGI */
void escm_error(const char *fmt, ...);
char ** tokenize_cmd(const char *cmd);

/* fork.c */
FILE *escm_popen(char * const argv[]);
int escm_pclose(FILE *fp);

/* memory allocation */
void *escm_malloc(size_t size);
void *escm_realloc(void *ptr, size_t size);
#define XMALLOC(type, n) ((type *) escm_malloc((n) * (sizeof(type))))
#define XREALLOC(type, p, n) ((type *) escm_realloc((p), (n) * (sizeof(type))))

/* redirection */
void escm_redirect(int from, int to);

#endif /* not MISC_H */
/* end of misc.h */
