#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "proxy.hpp"

void become_deamon() {
  //use fork() to create a child process
  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "failed to fork" << std::endl;
  }
  else if (pid != 0) {
    //exit parent process
    exit(0);
  }

  // dissociate from controlling tty(use setsid())
  if (setsid() == -1) {
    std::cerr << "failed to become a session leader" << std::endl;
  }

  //point stdin/stdout/stderr at /dev/null( use dup2( ))

  int fd1 = open("/dev/null", O_RDONLY);
  int fd2 = open("/dev/null", O_WRONLY);
  int fd3 = open("dev/null", O_RDWR);
  dup2(STDIN_FILENO, fd1);
  dup2(STDOUT_FILENO, fd2);
  dup2(STDERR_FILENO, fd3);

  //Chdir to "/"
  if (chdir("/") == -1) {
    std::cerr << "failed to change working directory" << std::endl;
  }
  //set umask to 0
  umask(0);
  // Fork again, allowing the parent process to terminate.
  signal(SIGHUP, SIG_IGN);
  pid = fork();
  if (pid == -1) {
    std::cerr << "failed to fork" << std::endl;
  }
  else if (pid != 0) {
    exit(0);
  }
}
int main(void) {
  become_deamon();
  Proxy proxy;
  proxy.run();
  return EXIT_SUCCESS;
}
