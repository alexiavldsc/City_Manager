#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
volatile sig_atomic_t stop=0;
void handle_sigint(int sig){
  (void)sig; //pt a evita warning-ul
  char msg[]="Sigint a fost primit\n";
  write(STDOUT_FILENO, msg, sizeof(msg)-1);
  stop=1;
}
void handle_sigusr1(int sig){
  (void)sig; //pt a evita warning-ul
  char msg[]="Raport nou adaugat\n";
  write(STDOUT_FILENO, msg, sizeof(msg)-1);
}
int main(){
  //pt a scrie pid-ul
  int fd=open(".monitor_pid", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd<0){
    perror("Eroare la crearea fisierului .monitor_pid");
    exit(1);
  }
  char buf[32];
  pid_t pid=getpid();
  int len=snprintf(buf, sizeof(buf), "%d\n", pid);
  write(fd, buf, len);
  close(fd);
  //setam semnalele
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler=handle_sigint;
  sigaction(SIGINT, &sa, NULL);
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler=handle_sigusr1;
  sigaction(SIGUSR1, &sa, NULL);
  while(!stop){ //asteptam semnalele
    pause();
  }
  unlink(".monitor_pid"); //pt a sterge fisierul
  return 0;
}
