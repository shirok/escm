/* fork.c - `pipe', `fork' and `exec' version server.
 * $Id$
 *
 * Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include <errno.h>
#include <signal.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif /* HAVE_SYS_WAIT_H */
#include "escm.h"

static char *def_argv[] = { ESCM_BACKEND_ARGV, NULL };

static void
sigterm(void)
{
  kill(0, SIGTERM);
}

FILE *
escm_popen(const char * prog)
{
  int fd[2];
  pid_t pid;

  if (pipe(fd) < 0) escm_error(NULL);
  if ((pid = fork()) < 0)  escm_error(NULL);
  if (pid > 0) { /* parent process */
    FILE *pipe;

    close(fd[0]); /* read */
    pipe = fdopen(fd[1], "w"); /* write */
    if (pipe == NULL) {
      int this_errno = errno;
      kill(pid, SIGTERM);
      errno = this_errno;
      escm_error(NULL);
    }
    return pipe;
  } else { /* child process */
    char **argv = NULL;

    atexit(sigterm);
    if (prog) {
      char *p;
      char *str;
      int i, n = 0;

      str = (char *)malloc(1 + strlen(prog));
      if (!str) escm_error(NULL);
      strcpy(str, prog);
      p = strtok(str, " \t");
      for (i = 0; /**/; i++, p = strtok(NULL, " \t")) {
	if (i == n) {
	  n += 4;
	  argv = (char **)realloc(argv, sizeof(char*) * n);
	  if (!argv) escm_error(NULL);
	}
	argv[i] = p;
	if (p == NULL) break;
      }
    } else {
      argv = def_argv;
    }

    close(fd[1]); /* write */
    /* connect fd[0] to the child process's stdin */
    if (dup2(fd[0], 0) < 0) escm_error(NULL);
    /* Invoke the interpreter */
    if (argv[0]) execvp(argv[0], argv);
    /* never reached if successful. */
    escm_error(gettext("can't invoke - %s"), argv[0]);
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
