/* meta_arg.h for aescm.
 * $Id$
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifndef META_ARG_H
#define META_ARG_H

#include "escm.h"
int meta_args(int *pargc, char ***pargv);
int meta_skip_shebang(FILE *fp);

#endif /* META_ARG_H */
/* end of meta_arg.h */
