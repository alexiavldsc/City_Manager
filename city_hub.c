#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
static pid_t hub_mon_pid = -1;
static void hub_mon(){ //ruleaza in procesul copil
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa)); // pt a curata structura
  sa.sa_handler=SIG_IGN;    //setam actiunea pe ignorare
  sigemptyset(&sa.sa_mask);
  sa.sa_flags=0;
  if(sigaction(SIGINT, &sa, NULL)<0){
    perror("Eroare la sigaction in hub_mon");
    _exit(1);
  }
  int mon_pipe[2];
  if(pipe(mon_pipe)<0){
    perror("[hub_mon] eroare pipe");
    _exit(1);
  }
  pid_t mon_pid=fork();
  if(mon_pid<0){
    perror("[hub_mon] eroare fork monitor");
    _exit(1);
  }
  if(mon_pid == 0){
    close(mon_pipe[0]); //nu citim din pipe
    if(dup2(mon_pipe[1], STDOUT_FILENO)<0){ //stdout->pipe
      perror("dup2 esuat in monitor");
      _exit(1);
    }
    close(mon_pipe[1]);
    execl("./monitor_reports", "monitor_reports", NULL);
    perror("[monitor] exec esuat");
    _exit(1);
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
  if(n == 0){
    printf("\n[hub_mon] Conexiunea cu monitorul s-a inchis.\n");
    fflush(stdout);
  }
  close(mon_pipe[0]);
  waitpid(mon_pid, NULL, 0);
  printf("\n[hub_mon] Monitorul s-a oprit.\n");
  fflush(stdout);
  _exit(0);
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
    _exit(0);
  }
  //parinte=city_hub, nu asteaptam hub_mon
  hub_mon_pid=pid;
  printf("Monitor pornit. (hub_mon PID=%d)\n", hub_mon_pid);
}
static void calculate_scores(char* args){
  char* districts[32];
  int nd=0;
  char* tok=strtok(args, " ");
  while(tok && nd<32){
    districts[nd++]=tok;
    tok=strtok(NULL, " ");
  }
  if(nd == 0){
    printf("Specifica cel putin un district.\n");
    return;
  }
  int pipes[32][2];
  pid_t pids[32];
  for(int i=0;i<32;i++){
    pids[i]=-1; //pt a sti ce procese au esuat
  }
  for(int i=0;i<nd;i++){ //lansam toti scorerii simultan
    if(pipe(pipes[i])<0){
      perror("pipe");
      continue; //sarim districtul curent
    }
    pids[i]=fork();
    if(pids[i]<0){
      perror("fork");
      //inchidem pipe-ul creat pt a nu avea memory leak
      close(pipes[i][0]);
      close(pipes[i][1]);
      continue; //sarim districtul curent
    }
    if(pids[i] == 0){ //copil
      close(pipes[i][0]);
      if(dup2(pipes[i][1], STDOUT_FILENO)<0){
	perror("dup2 esuat in calculate_scores");
	_exit(1);
      }
      close(pipes[i][1]);
      execl("./scorer", "scorer", districts[i], NULL);
      perror("exec scorer");
      _exit(1);
    }
    //parinte
    close(pipes[i][1]); //trebuie inchisa scrierea
  }
  for(int i=0;i<nd;i++){ //asteptam copiii
    if(pids[i]<=0){
      continue;
    }
    printf("Scoruri pentru %s:\n", districts[i]);
    char buf[1024];
    ssize_t n;
    while((n=read(pipes[i][0], buf, sizeof(buf)-1))>0){
      buf[n]='\0';
      printf("%s", buf);
    }
    close(pipes[i][0]);
    waitpid(pids[i], NULL, 0);
  }
}
static void stop_monitor(){
    if(hub_mon_pid <= 0 || kill(hub_mon_pid, 0) != 0){
        printf("Monitorul nu ruleaza.\n");
        return;
    }
    int fd=open(".monitor_pid", O_RDONLY);
    if(fd>=0){
        char buf[32]={0};
        read(fd, buf, sizeof(buf)-1);
        close(fd);
        pid_t mon_pid=(pid_t)atoi(buf);
        if(mon_pid>0 && kill(mon_pid, 0) == 0){
            kill(mon_pid, SIGINT);
        }
    }
    waitpid(hub_mon_pid, NULL, 0);
    hub_mon_pid=-1;
    printf("Monitor oprit.\n");
}
int main() {
  char line[512];
  printf("city_hub:\n");
  printf("Comenzi: start_monitor, stop_monitor, calculate_scores <districte>, quit\n");
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
    else if(strncmp(line, "calculate_scores ", 17) == 0){
      calculate_scores(line + 17);
    }
    else if(strcmp(line, "stop_monitor") == 0){
      stop_monitor();
    }
    else if(strlen(line) == 0){
      continue;
    }
    else{
      printf("Comanda necunoscuta: '%s'\n", line);
    }
  }
  if(hub_mon_pid>0 && kill(hub_mon_pid, 0) == 0){
    stop_monitor();
  }
  printf("city_hub oprit.\n");
  return 0;
}
