/* fork.h - invoke a backend interpeter
 * $Id$
 * Copyright (c) 2004 TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifndef FORK_H
#define FORK_H 1

#include <stdio.h>
FILE* escm_popen(char* const argv[]);
int escm_pclose(FILE* fp);
void escm_redirect(int from, int to);

#endif /* not FORK_H */
/* end of fork.h */
