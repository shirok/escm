/* fork.c - `pipe', `fork' and `exec' version server.
 * $Id$
 *
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "config.h"
#include "../cgi.h"

/* pipe = escm_connect(argv) - connect with the backend interpreter.
 */
FILE *
escm_connect(char *const argv[])
{
  int fd[2];
  pid_t pid;

  if (pipe(fd) < 0) cgi_error("can't make a pipe");
  if ((pid = fork()) < 0)  cgi_error("can't fork");
  if (pid > 0) { /* parent process */
    FILE *pipe;

    close(fd[0]); /* read */
    pipe = fdopen(fd[1], "w"); /* write */
    if (pipe == NULL) {
      int this_errno = errno;
      kill(pid, SIGTERM);
      errno = this_errno;
      cgi_error("can't really open the pipe");
    }
    return pipe;
  } else { /* child process */
    close(fd[1]); /* write */
    /* connect fd[0] to the child process's stdin */
    if (dup2(fd[0], 0) < 0) cgi_error("can't redirect stdin");
    /* connect the child process's stderr to stdout if in a CGI script */
    if (is_cgi() && dup2(1, 2) < 0) cgi_error("can't redirect stderr");
    /* invoke the interpreter */
    execvp(argv[0], argv);
    /* never reached if successful */
    cgi_error("can't invoke %s", argv[0]);
    return NULL; /* dummy */
  }
}

/* escm_disconnect(pipe) - disconnect from the backend interpreter.
 */
int
escm_disconnect(FILE *pipe)
{
  int status;

  fflush(pipe);
  fclose(pipe);
  /* wait till the child process exits. */
  wait(&status);
  return 0;
}
/* end of fork.c */
