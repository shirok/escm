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

/* misc.c */
void escm_html_header(const struct escm_lang *lang, FILE *outp);
void escm_text_header(const struct escm_lang *lang, FILE *outp);
void escm_error(const char *fmt, ...);

/* fork.c */
FILE *escm_popen(char * const argv[]);
int escm_pclose(FILE *fp);
void escm_redirect(int from, int to);

/* cmdline.c */
char ** parse_cmdline(const char *cmd);
FILE* parse_shebang(const char* prog, const char* script, int* pargc, char*** pargv);
void * xmalloc(size_t size);
void * xrealloc(void *ptr, size_t size);
#define XMALLOC(type, n) ((type *) xmalloc((n) * (sizeof(type))))
#define XREALLOC(type, p, n) ((type *) xrealloc((p), (n) * (sizeof(type))))
#define XERROR escm_error
#define escm_malloc xmalloc
#define escm_realloc xrealloc

char *escm_cgi;
#endif /* not MISC_H */
/* end of misc.h */
