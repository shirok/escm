/* escmcgi.h - functions for the CGI tool.
 * $Id$
 * Copyright (c) 2003-2004 TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifndef ESCMCGI_H
#  define ESCMCGI_H 1

#include "escm.h"

void escm_header(const struct escm_lang *lang, FILE *outp);
int escm_query_string(const struct escm_lang *lang, FILE *outp);
void escm_skip_shebang(FILE *inp);
void escm_add_header(const struct escm_lang *lang, FILE *inp, FILE *outp);

#endif /* ESCMCGI_H */
