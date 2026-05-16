#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
static pid_t hub_mon_pid = -1;
static void hub_mon(){ //ruleaza in procesul copil
  int mon_pipe[2];
  if(pipe(mon_pipe)<0){
    perror("[hub_mon] eroare pipe");
    exit(1);
  }
  pid_t mon_pid=fork();
  if(mon_pid<0){
    perror("[hub_mon] eroare fork monitor");
    exit(1);
  }
  if(mon_pid == 0){
    close(mon_pipe[0]); //nu citim din pipe
    dup2(mon_pipe[1], STDOUT_FILENO); //stdout->pipe
    close(mon_pipe[1]);
    execlp("./monitor_reports", "monitor_reports", NULL);
    perror("[monitor] exec esuat");
    exit(1);
  }
  //hub_mon citeste din pipe si afiseaza
  close(mon_pipe[1]); //nu scriem in pipe
  char buf[256];
  ssize_t n;
  while((n=read(mon_pipe[0], buf, sizeof(buf)-1))>0){
    buf[n]='\0';
    if(strncmp(buf, "MSG_ERROR:", 10) == 0){
      printf("\n[MONITOR EROARE] %s", buf+10);
      fflush(stdout);
      break; //monitorul nu a pornit, oprim
    }
    else if(strncmp(buf, "MSG_EXIT:", 9) == 0){
      printf("\n[MONITOR STOP] %s", buf+9);
      fflush(stdout);
      break;
    }
    else if(strncmp(buf, "MSG_INFO:", 9) == 0){
      printf("\n[MONITOR] %s", buf+9);
      fflush(stdout);
    }
    else{
      printf("\n[MONITOR] %s", buf);
      fflush(stdout);
    }
    printf("> ");
    fflush(stdout);
  }
  close(mon_pipe[0]);
  waitpid(mon_pid, NULL, 0);
  printf("\n[hub_mon] Monitorul s-a oprit.\n");
  fflush(stdout);
  exit(0);
}
static void start_monitor(){
  //verificam daca hub_mon mai e viu
  if(hub_mon_pid>0 && kill(hub_mon_pid, 0) == 0){
    printf("Monitorul deja ruleaza (hub_mon PID=%d).\n", hub_mon_pid);
    return;
  }
  pid_t pid=fork();
  if(pid<0){
    perror("eroare fork hub_mon");
    return;
  }
  if(pid == 0){
    //copil=hub_mon
    hub_mon();
    exit(0);
  }
  //parinte=city_hub, nu asteaptam hub_mon
  hub_mon_pid=pid;
  printf("Monitor pornit. (hub_mon PID=%d)\n", hub_mon_pid);
}
int main() {
  char line[512];
  printf("city_hub:\n");
  printf("Comenzi: start_monitor, quit\n");
  while(1){
    printf("> ");
    fflush(stdout);
    if(!fgets(line, sizeof(line), stdin)){
      break;
    }
    line[strcspn(line, "\n")]='\0';
    if(strcmp(line, "quit") == 0){
      break;
    }
    else if(strcmp(line, "start_monitor") == 0){
      start_monitor();
    }
    else if(strlen(line) == 0){
      continue;
    }
    else{
      printf("Comanda necunoscuta: '%s'\n", line);
    }
  }
  if(hub_mon_pid>0 && kill(hub_mon_pid, 0) == 0){
    waitpid(hub_mon_pid, NULL, 0);
  }
  printf("city_hub oprit.\n");
  return 0;
}
