#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
typedef struct{
  float latitude;
  float longitude;
}Coordinates;
enum Severity_level{
  minor=1,
  moderate=2,
  critical=3
};
typedef struct{
  int id;
  char inspector[25];
  Coordinates coordinates;
  char category[10];
  enum Severity_level level;
  time_t timestamp;
  char description[100];
}Report;
typedef struct{
  char nume[25];
  int scor;
}InspScore;
int main(int argc, const char** argv){
  if(argc!=2){
    fprintf(stderr, "Eroare: Trebuie specificat districtul!\n");
    return 1;
  }
  char cale[256];
  snprintf(cale, sizeof(cale), "%s/reports.dat", argv[1]);
  int fd=open(cale, O_RDONLY);
  if(fd<0){
    perror("Nu s-au gasit rapoarte pt acest district");
    return 0;
  }
  Report rep;
  InspScore scoruri[100];
  int nr_inspectori=0;
  while(read(fd, &rep, sizeof(Report)) == sizeof(Report)){
    int gasit=0;
    for(int i=0;i<nr_inspectori;i++){
      if(strcmp(scoruri[i].nume, rep.inspector) == 0){
	scoruri[i].scor+=(int)rep.level;
	gasit=1;
	break;
      }
    }
    if(!gasit && nr_inspectori<100){
      strncpy(scoruri[nr_inspectori].nume, rep.inspector, sizeof(scoruri[nr_inspectori].nume)-1);
      scoruri[nr_inspectori].nume[24]='\0';
      scoruri[nr_inspectori].scor=(int)rep.level;
      nr_inspectori++;
    }
  }
  if(nr_inspectori == 0){
    printf("Niciun report procesat\n");
  }
  else{
    for(int i=0;i<nr_inspectori;i++){
      printf("Inspector: %s ; Workload: %d\n", scoruri[i].nume, scoruri[i].scor);
    }
  }
  close(fd);
  return 0;
}
