/* meta_arg.h for aescm.
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifndef META_ARG_H
#define META_ARG_H
int meta_args(int *pargc, char ***pargv);
int meta_args_replace(int *pargc, char ***pargv, const char *filename, int from);
int meta_skip_shebang(FILE *fp);

/* values to be returned */
#define META_ARGS_OK 0
#define META_ARGS_NOT -1

#ifdef ESCM
#include "escm.h"
#include "misc.h"
#define xerror1 escm_error
#define xerror2 escm_error
#define xprog escm_prog
#define xfile escm_file
#define xlineno escm_lineno
#endif /* ESCM */
#endif /* META_ARG_H */
/* end of meta_arg.h */
