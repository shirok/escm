/* misc.h - non public header for escm
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifndef MISC_H
#define MISC_H 1

/* Used to make error messages. */
extern const char *cgi_prog;
extern const char *cgi_file;
extern int cgi_header_flag;

void cgi_error(const char *fmt, ...);

#endif /* not MISC_H */
/* end of misc.h */
