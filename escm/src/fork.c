/* fork.c - `pipe', `fork' and `exec' version server.
 * $Id$
 *
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#ifdef HAVE_STRING_H
# include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */
#include "escm.h"

static char *scm_argv[] = { ESCM_BACKEND_ARGV, NULL };

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
    char *my_argv[16];
    char *p;
    int i;
    char **argv;

    if (prog) {
      argv = my_argv;
      strncpy(str, prog, 127);
      str[127] = '\0';
      p = strtok(str, " \t");
      if (p == NULL) escm_error("Invalid program name: %s", prog);
      for (i = 0; i < 16; i++, p = strtok(NULL, " \t")) {
	argv[i] = p;
	if (p == NULL) break;
      }
      if (i == 16) escm_error("Too many arguments: %s", prog);
    } else {
      argv = scm_argv;
    }

    close(fd[1]); /* write */
    /* connect fd[0] to the child process's stdin */
    if (dup2(fd[0], 0) < 0)
      escm_error("can't redirect stdin");
    /* invoke the interpreter */
    execvp(argv[0], argv);
    /* never reached if successful. */
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
  if (WIFEXITED(status)) return WEXITSTATUS(status);
  else return -1;
}
/* end of fork.c */
