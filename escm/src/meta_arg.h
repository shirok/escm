/* meta_arg.h for aescm.
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifndef META_ARG_H
#define META_ARG_H
int meta_args(int *pargc, char ***pargv);
int meta_skip_shebang(FILE *fp);
#define META_ARGS_OK 0
#define META_ARGS_NOT -1
#define META_ARGS_ERRNO_ERROR -2
#define META_ARGS_SYNTAX_ERROR -3
#endif /* META_ARG_H */
/* end of meta_arg.h */
