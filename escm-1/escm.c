/*
 * eScheme - Embedded Scheme code processor.
 *
 *  Copyright (c) 2000-2001 Shiro Kawai  shiro@acm.org
 *
 *  Permission is hereby granted, free of charge, to any person
 *  obtaining a copy of this software and associated documentation
 *  files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify,
 *  merge, publish, distribute, sublicense, and/or sell copies of
 *  the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 *  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 *  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 *  OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 *  $Id$
 */

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>             /* for strerror() */
#include <signal.h>
#include <sys/wait.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

#ifndef INTERPRETER
#define INTERPRETER "snow"
#endif

#ifndef VERSION
#define VERSION "0.2.1"
#endif

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 1024
#endif

static char *get_interpreter_name(FILE *, int*, int);
static void preprocess(FILE*, FILE*, int, int);
static void parameters(FILE*, char*, char*, char*);
static void usage(int);
static void showversion(void);
static void report_error(int, char *, ...);
static char *addstr(char *, char *, int);
static int tokenize(char* str, char*** token_array);

int main(int argc, char **argv)
{
    int opt;
    int nextchar;
    int preprocess_only = FALSE;
    int cgi_mode = FALSE;
    char *outfile = NULL;
    char *infile = NULL;
    char *interpreter = NULL, *intp;
    char **int_argv = NULL;
    FILE *infp = NULL, *outfp = NULL;
    char *extraexpr = NULL;

    opterr = 0;                 /* tell getopt not to print error msg */
    while ((opt = getopt(argc, argv, "Ecvo:i:e:")) >= 0) {
        switch (opt) {
        case 'E': preprocess_only = TRUE; continue;
        case 'c': cgi_mode = TRUE; continue;
        case 'v': showversion();
        case 'e': extraexpr = addstr(extraexpr, optarg, cgi_mode); continue;
        case 'o': outfile = optarg; continue;
        case 'i': interpreter = optarg; continue;
        case '?': usage(1);
        }
    }
    
    if (optind < argc) {
        if (optind == argc - 1) {
            infile = argv[optind];
        } else {
            usage(1);
        }
    }

    if (infile) {
        infp = fopen(infile, "r");
        if (infp == NULL) {
            report_error(cgi_mode, "couldn't open input file `%s': %s\n",
                         infile, strerror(errno));
        }
    } else {
        infp = stdin;
    }
    
    if (outfile) {
        outfp = fopen(outfile, "w");
        if (outfp == NULL) {
            report_error(cgi_mode, "couldn't open output file `%s': %s\n",
                         outfile, strerror(errno));
        }
    } else {
        outfp = stdout;
    }

    /*
     * See if interpreter is specified in the beginning of the input file.
     */
    intp = get_interpreter_name(infp, &nextchar, cgi_mode);
    if (interpreter == NULL) interpreter = intp; /* -i option has precedence */

    /*
     * Now, process the input.
     */
    if (preprocess_only) {
        parameters(outfp, infile, outfile, extraexpr);
        preprocess(infp, outfp, nextchar, cgi_mode);
    } else {
        int p[2];
        int pid;
        if (pipe(p) < 0) {
            report_error(cgi_mode, "couldn't open a pipe: %s\n",
                         strerror(errno));
        }
        if ((pid = fork()) < 0) {
            report_error(cgi_mode, "couldn't fork: %s\n", strerror(errno));
        } else if (pid == 0) {
            /* child process
               adjust stdin and stdout, then exec the interpreter. */
            close(p[1]);
            if (dup2(p[0], 0) < 0) {
                report_error(cgi_mode, "dup failed: %s\n", strerror(errno));
            }
            if (outfp != stdout) {
                int outfd = fileno(outfp);
                if (dup2(outfd, 1) < 0) {
                    report_error(cgi_mode, "dup failed: %s\n",
                                 strerror(errno));
                }
            }
            if (cgi_mode) {
                /* redirect stderr of child process to stdout */
                int outfd = fileno(outfp);
                if (dup2(outfd, 2) < 0) {
                    report_error(cgi_mode, "dup failed: %s\n",
                                 strerror(errno));
                }
            }

            if (ENOMEM == tokenize(interpreter, &int_argv)) {
                report_error(cgi_mode, "Out of memory");
            }
            execvp(int_argv[0], int_argv);
            report_error(cgi_mode, "can't execute interpreter %s: %s\n",
                         interpreter, strerror(errno));
        } else {
            /* parent process */
            int stat;
            close(p[0]);
            outfp = fdopen(p[1], "w");
            if (outfp == NULL) {
                int reason = errno;
                kill(pid, SIGTERM);
                report_error(cgi_mode, "fdopen failed: %s\n", strerror(reason));
            }
            parameters(outfp, infile, outfile, extraexpr);
            preprocess(infp, outfp, nextchar, cgi_mode);
            fflush(outfp);
            fclose(outfp);
            wait(&stat);
        }
    }
    exit(0);
}

/*
 * Exception handling...
 */

static void usage(int code)
{
    fputs("Usage: escm [-Ecv][-i interpreter][-o outfile][-e expr] [infile]\n", stderr);
    exit(code);
}

static void showversion()
{
    fprintf(stderr, "escm version %s\nCopyright(c) 2000-2001 by Shiro Kawai (shiro@acm.org)\n",
            VERSION);
    fprintf(stderr, "  Default interpreter: %s\n", INTERPRETER);
    exit(0);
}

static void report_error(int cgi_mode, char *fmt, ...)
{
    va_list ap;

    if (cgi_mode) {
        puts("Content-type: text/html\n");
        puts("<html><head><title>Script error</title></head>");
        puts("<body><h1>Script error</h1><pre>");
    }
    va_start(ap, fmt);
    vfprintf((cgi_mode? stdout : stderr), fmt, ap);
    va_end(ap);
    if (cgi_mode) {
        puts("\n</pre></body></html>");
    }
    exit(cgi_mode? 0 : 1);
}

/*
 * Find interpreter name
 *
 *  (1) Skip #!-line, if any.
 *  (2) If the next line begins with #?, take 
 */

static char *get_interpreter_name(FILE *in, int *nextchar, int cgi_mode)
{
    int c, c1;
    static char namebuf[MAX_PATH_LEN+1];
    int interpreter_found = FALSE;
    
    /* Skip the first #! line */
    if ((c = getc(in)) == '#') {
        if ((c1 = getc(in)) == '!') {
            while ((c = getc(in)) != '\n' && c != EOF) {
                ;/*skip*/
            }
            c = getc(in);
        } else {
            ungetc(c1, in);
        }
    }

    /* Is the next line begins with '#?' ? */
    if (c == '#') {
        if ((c1 = getc(in)) == '?') {
            int count = 0;
            char *ptr = namebuf;

            do { c = getc(in); } while (c == ' ');
            if (c != '\n' && c != EOF) {
                do { *ptr++ = c; count++; c = getc(in); }
                while (c != '\n' && c != EOF && count < MAX_PATH_LEN);
                if (count >= MAX_PATH_LEN) {
                    report_error(cgi_mode, "interpreter path too long\n");
                }
                *ptr = '\0';
                interpreter_found = TRUE;
            }
            c = getc(in);
        } else {
            ungetc(c1, in);
        }
    }
    if (!interpreter_found) {
        strcpy(namebuf, INTERPRETER);
    }

    *nextchar = c;
    return namebuf;
}

/* push string */
static char *addstr(char *s1, char *s2, int cgi_mode)
{
    if (s1) {
        char *r = (char *)malloc(strlen(s1) + strlen(s2) + 2);
        if (r == NULL) report_error(cgi_mode, "Out of memory");
        strcpy(r, s1);
        strcat(r, "\n");
        strcat(r, s2);
        free(s1);
        return r;
    } else {
        char *r = strdup(s2);
        if (r == NULL) report_error(cgi_mode, "Out of memory");
        return r;
    }
}

/*
 * Preprocessor
 */

/* send preset parameters to Scheme interpreter */
static void parameters(FILE *out, char *infile, char* outfile, char *extra)
{
    fprintf(out, ";; ESCM parameters\n");
    if (infile) {
        fprintf(out, "(define *escm-input-file* \"%s\")\n", infile);
    } else {
        fprintf(out, "(define *escm-input-file* #f)\n");
    }
    if (outfile) {
        fprintf(out, "(define *escm-output-file* \"%s\")\n", outfile);
    } else {
        fprintf(out, "(define *escm-output-file* #f)\n");
    }
    fprintf(out, "(define *escm-version* \"%s\")\n", VERSION);
    if (extra) {
        fprintf(out, ";; Extra expression(s) provided in command line\n");
        fprintf(out, "%s\n", extra);
    }
}

#define LITERAL_PREFIX   "(display \""
#define LITERAL_POSTFIX  "\")"
#define EXPR_PREFIX      "(display "
#define EXPR_POSTFIX     ")"

static void preprocess(FILE *in, FILE *out, int nextchar, int cgi_mode)
{
    enum {
        IN_LITERAL,
        IN_SCHEME,
        IN_EXPR
    } status = IN_LITERAL;
    int c = nextchar, c1;

    fputs(";; ESCM translated part follows\n", out);
    fputs(LITERAL_PREFIX, out);

    do {
        if (status == IN_LITERAL) {
            switch (c) {
            case '<':
                c1 = getc(in);
                if (c1 == '?') {
                    int c2 = getc(in);
                    if (c2 == '=') {
                        fputs(LITERAL_POSTFIX, out);
                        fputs(EXPR_PREFIX, out);
                        status = IN_EXPR;
                        continue;
                    } else if (c2 == EOF) {
                        fputs("<?", out); /* unterminated <? */
                        break;
                    } else {
                        fputs(LITERAL_POSTFIX, out);
                        status = IN_SCHEME;
                        continue;
                    }
                } else if (c1 == EOF) {
                    fputc(c, out);
                    break;
                } else {
                    fputc(c, out);
                    fputc(c1, out);
                    continue;
                }
            case '"':;          /* FALLTHROUGH */
            case '\\':
                fputc('\\', out);
                fputc(c, out);
                continue;
            default:
                fputc(c, out);
                continue;
            }
        } else {
            if (c == '!') {
                c1 = getc(in);
                if (c1 == '>') {
                    /* End of Scheme part. */
                    fputc('\n', out);/* extra newline to terminate comment (if any)  */
                    if (status == IN_EXPR) fputs(EXPR_POSTFIX, out);
                    fputs(LITERAL_PREFIX, out);
                    status = IN_LITERAL;
                    continue;
                } else if (c == EOF) {
                    fputc(c, out);
                    break;
                } else {
                    fputc(c, out);
                    fputc(c1, out);
                    continue;
                }
            } else {
                fputc(c, out);
                continue;
            }
        }
    }
    while ((c = getc(in)) != EOF);

    if (status == IN_LITERAL) fputs(LITERAL_POSTFIX, out);
    else if (status == IN_EXPR) fputs(EXPR_POSTFIX, out);
    fputc('\n', out);
}

/*
 * Tokenizer for parsing #? line.  Provided by HATTA Shuzo (hattas@debian.org)
 *   this func will destruct contents of 'str'
 */
int tokenize(char* str, char*** token_array)
{
    char* start;
    char delimit;
    int cont_flag = 1;
    int cnt = 0;

    if (str == NULL) {
        return 0;
    }
    *token_array = (char**) malloc (sizeof(char*));
    if (*token_array == NULL) {
        return ENOMEM;
    }
    while (cont_flag) {

        /* skip whitespace */
        while(*str != '\0' && isspace(*str)) {
            str++;
        }
        if (*str == '\0') {
            break;
        }

        if (*str == '\'' || *str == '\"') {
            /* quoted param */
            delimit = *str;
            str++;
            start = str;
            while(*str != '\0' && *str != delimit) {
                str++;
            }
        } else {
            start = str;
            while(*str != '\0' && !isspace(*str)) {
                str++;
            }
        }
        if (*str != '\0') {
            *str = '\0';
            str++;
        }
        if (*str == '\0') {
            cont_flag = 0;
        }
        *token_array = (char**) realloc (*token_array,
                                         sizeof(char*) * (cnt + 2));
        if (*token_array == NULL) {
            return ENOMEM;
        }
        (*token_array)[cnt] = start;
        cnt++;
    }
    (*token_array)[cnt] = NULL;
    return 0;
}
