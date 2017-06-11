#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

void die(const char *s) {
  perror(s);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[], char *envp[]) {
  int pid, status, size, filedes[2];
  fd_set fds;
  struct timeval t;
  char buf[64];
  struct user_regs_struct regs;

  if (pipe(filedes) == -1) {
    die("pipe");
  }

  pid = fork();

  if (pid == -1) {
    die("fork");
  }

  /* child process */
  if (pid == 0) {
    if (close(1) == -1) {
      die("child close(1)");
    }
    if (dup2(filedes[1], 1) == -1) {
      die("child dup2");
    }
    if (close(filedes[0]) == -1) {
      die("child filedes[0]");
    }
    if (close(filedes[1]) == -1) {
      die("child filedes[1]");
    }
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
      die("child ptrace");
    }
    if (execve(argv[1], argv + 1, envp) == -1) {
      die("child execve");
    }
  }

  /* parent process*/
  printf("pid=%d\n", pid);

  while (1) {
    if (waitpid(pid, &status, 0) == -1) {
      die("waitpid");
    }

    if (WIFEXITED(status)) {
      break;
    }

    FD_ZERO(&fds);
    FD_SET(filedes[0], &fds);
    t.tv_sec = 0;
    t.tv_usec = 0;
    if (select(filedes[0] + 1, &fds, NULL, NULL, &t) == -1) {
      die("select");
    }
    if (FD_ISSET(filedes[0], &fds)) {
      size = read(filedes[0], buf, sizeof(buf));
      buf[size] = '\0';
      if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1) {
        die("ptrace PTRACE_GETREGS");
      }
      fprintf(stderr, "Wrote: RIP=%016llx, %s\n", regs.rip, buf);
      if (ptrace(PTRACE_CONT, pid, NULL, SIGABRT) == -1) {
        die("ptrace PTRACE_CONT");
      }
      if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1) {
        die("ptrace PTRACE_DETACH");
      }
      break;
    }
    ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);
  }
  
  return 0;
}
