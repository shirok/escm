/* parse.h - parse a string as command line
 * $Id$
 */
#ifndef PARSE_H
#define PARSE_H 1
extern char ** command_line(char *line, int *argc);
extern char ** shebang(char **argv, int *argc);
#ifdef MAYBE_CGI
#include "cgi.h"
#define xerror(str) cgi_error(str)
#endif /* MAYBE_CGI */
#endif /* PARSE_H */
/* end of parse.h */
