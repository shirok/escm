/* cmdline.h - parse a string as command line
 * $Id$
 * Copyright (c) 2004 TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifndef CMDLINE_H
#define CMDLINE_H 1

char** parse_cmdline(const char *cmd);
FILE* parse_shebang(const char* prog, const char* script, int* pargc, char*** pargv);
void* xmalloc(size_t size);
void* xrealloc(void *ptr, size_t size);
#define XMALLOC(type, n) ((type *) xmalloc((n) * (sizeof(type))))
#define XREALLOC(type, p, n) ((type *) xrealloc((p), (n) * (sizeof(type))))

#endif /* not CMDLINE_H */
/* end of cmdline.h */
