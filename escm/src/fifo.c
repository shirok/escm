/* fifo.c - CGI program using gauche-fifoserv
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include "escm.h"
#include "escmcgi.h"

#define STATEDIR       "/home/tagga/tmp/cgi"
#define BASENAME       "sample"
#define FIFO_IN        STATEDIR "/" BASENAME ".in"
#define FIFO_OUT_FMT   STATEDIR "/" BASENAME ".%d"
#define BINDIR         "/home/tagga/bin"
#define SERVER         BINDIR "/" "fifo-server"
#define MAXLEN (PATH_MAX * 2)
#define MAXWAIT 10

void
my_syserr(void)
{
  perror("aescm(fifo)");
  exit(EXIT_FAILURE);
}
void
my_error(const char *msg)
{
  fputs("aescm(fifo): ", stderr);
  fputs(msg, stderr);
  fputs("\n", stderr);
  exit(EXIT_FAILURE);
}

/* check if the server's input named pipe, FIFO.in for example,
   exists; if it is not the case, invoke the server and wait till
   FIFO.in appears; */
void
check_server(const char *server, const char *fifo_in)
{
  if (access(fifo_in, W_OK) != 0) {
    char cmdline[MAXLEN];
    int i, ret;
    struct timespec req;

    ret = snprintf(cmdline, MAXLEN, "%s %s &", server, fifo_in);
    if (ret >= MAXLEN) my_error("command line too long");
    system(cmdline);
    req.tv_sec = 0;
    req.tv_nsec = 100000000; /* 0.1 sec */
    for (i = 0; i < MAXWAIT * 10; i++) {
      if (access(fifo_in, W_OK) == 0) return;
      nanosleep(&req, NULL);
    }
    my_error("can't invoke server");
  }
}
/* make a named pipe FIFO.PID through which the server sends the
   output, where PID is a unique identifier; */
void
make_fifo_out(char *fifo_out)
{
  int ret;
  ret = snprintf(fifo_out, PATH_MAX, FIFO_OUT_FMT, getpid());
  if (ret >= PATH_MAX) my_error("path too long");
  ret = mkfifo(fifo_out, 0600);
  if (ret != 0) my_syserr();
}

/* open and lock FIFO.in */
FILE *
open_and_lock(const char *fifo_in, struct flock *lock)
{
  int fd;
  FILE *fp;

  lock->l_type = F_WRLCK;
  lock->l_whence = SEEK_SET;
  lock->l_start = 0;
  lock->l_len = 0;
  fd = open(fifo_in, O_WRONLY);
  if (fd < 0) my_syserr();
  fcntl(fd, F_SETLKW, lock);
  fp = fdopen(fd, "w");
  if (fp == NULL) my_syserr();
  return fp;
}
/* unlock and close FIFO.in */
void
unlock_and_close(FILE *fp, struct flock *lock)
{
  lock->l_type = F_UNLCK;
  fcntl(fileno(fp), F_SETLKW, lock);
  fclose(fp);
}

/* read the output of the server from FIFO.PID */
void
copy_fifo_out(const char *fifo_out)
{
  char buf[BUFSIZ];
  FILE *fp;
  fp = fopen(fifo_out, "r");
  if (fp == NULL) my_syserr();
  while (fgets(buf, BUFSIZ, fp) != NULL) {
    fputs(buf, stdout);
  }
  fclose(fp);
}

extern struct escm_lang deflang;
int
main(int argc, char *argv[])
{
  char fifo_out[PATH_MAX];
  struct flock lock;
  char *infile = NULL;
  FILE *inp;
  FILE *outp;

  if (argc == 1) {
    infile = getenv("PATH_TRANSLATED");    
  } else {
    infile = argv[1];
  }

  check_server(SERVER, FIFO_IN);
  make_fifo_out(fifo_out);
  outp = open_and_lock(FIFO_IN, &lock);

  fprintf(outp, deflang.init, fifo_out);
  escm_bind(&deflang, "escm_version", PACKAGE " " VERSION "(cgi)", outp);
  escm_bind(&deflang, "escm_input_file", infile, outp);
  escm_bind(&deflang, "escm_interpreter", SERVER, outp);
  if (!escm_query_string(&deflang, outp))
    my_error("inconsistent environment variables.\n");
  inp = fopen(infile, "r");
  if (inp == NULL) my_syserr();

  escm_skip_shebang(inp);
  escm_add_header(&deflang, inp, outp);

  if (!escm_preproc(&deflang, inp, outp))
    my_error("unterminated instruction.\n");
  fclose(inp);
  escm_finish(&deflang, outp);

  unlock_and_close(outp, &lock);
  copy_fifo_out(fifo_out);
  unlink(fifo_out);
  return 0;
}
/* fifo.c ends here. */
