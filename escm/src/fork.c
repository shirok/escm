/* fork.c - `pipe', `fork' and `exec' version server.
 * $Id$
 *
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include "escm.h"

FILE *
escm_popen(const char * prog)
{
  int fd[2];
  pid_t pid;

  if (pipe(fd) < 0) escm_error("can't make a pipe");
  if ((pid = fork()) < 0)  escm_error("can't fork");
  if (pid > 0) { /* parent process */
    FILE *pipe;

    close(fd[0]); /* read */
    pipe = fdopen(fd[1], "w"); /* write */
    if (pipe == NULL) {
      int this_errno = errno;
      kill(pid, SIGTERM);
      errno = this_errno;
      escm_error("can't really open the pipe");
    }
    return pipe;
  } else { /* child process */
    /* AD HOC VERSION */
    char str[128];
    char *argv[16];
    char *p;
    int i;

    strncpy(str, prog, 127);
    str[127] = '\0';
    p = strtok(str, " \t");
    if (p == NULL) escm_error("Invalid program name: %s", prog);
    for (i = 0; i < 16; i++, p = strtok(NULL, " \t")) {
      argv[i] = p;
      if (p == NULL) break;
    }
    if (i == 16) escm_error("Too many arguments: %s", prog);

    close(fd[1]); /* write */
    /* connect fd[0] to the child process's stdin */
    if (dup2(fd[0], 0) < 0)
      escm_error("can't redirect stdin");
    /* invoke the interpreter */
    execvp(argv[0], argv);
    /* never reached if successful */
    escm_error("can't invoke %s", argv[0]);
    return NULL; /* dummy */
  }
}

int
escm_pclose(FILE *pipe)
{
  int status;

  fflush(pipe);
  fclose(pipe);
  /* wait till the child process exits. */
  wait(&status);
  return 0;
}
/* end of fork.c */
