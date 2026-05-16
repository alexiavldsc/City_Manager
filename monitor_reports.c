#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
volatile sig_atomic_t stop=0;
void handle_sigint(int sig){
  (void)sig; //pt a evita warning-ul
  char msg[]="MSG_EXIT:Sigint primit, monitorul se opreste\n";
  write(STDOUT_FILENO, msg, sizeof(msg)-1);
  stop=1;
}
void handle_sigusr1(int sig){
  (void)sig; //pt a evita warning-ul
  char msg[]="MSG_INFO:Raport nou adaugat\n";
  write(STDOUT_FILENO, msg, sizeof(msg)-1);
}
int main(){
  //verificam daca ruleaza deja un monitor citing .monitor_pid
  int fd_check=open(".monitor_pid", O_RDONLY);
  if(fd_check>=0){
    char buf_check[32]={0};
    int bytes_read=read(fd_check, buf_check, sizeof(buf_check)-1);
    close(fd_check);
    if(bytes_read>0){
      pid_t existing_pid=atoi(buf_check);
      //verificam daca procesul cu acel PID chiar exista
      if(existing_pid>0 && kill(existing_pid, 0) == 0){
        char err_msg[128];
        int len=snprintf(err_msg, sizeof(err_msg), "MSG_ERROR:Monitorul ruleaza deja cu PID-ul %d\n", existing_pid);
        write(STDOUT_FILENO, err_msg, len);
        exit(1); 
      }
    }
  }
  //pt a scrie pid-ul propriu, nu ruleaza niciun monitor activ
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
  char startmsg[64];
  int slen=snprintf(startmsg, sizeof(startmsg), "MSG_INFO:Monitor pornit cu PID=%d\n", pid);
  write(STDOUT_FILENO, startmsg, slen);
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
