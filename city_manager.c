#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
typedef struct{
  float latitude;
  float longitude;
}Coordinates;
enum Security_level{
  minor=1,
  moderate=2,
  critical=3
};
typedef struct{
  int id;
  char inspector[25];
  Coordinates coordinates;
  char category[10];
  enum Security_level level;
  time_t timestamp;
  char description[100];
}Report;
void create_director(const char* nume_director){
  if(mkdir(nume_director, 0750)<0){
    perror("Eroare la crearea directorului. Poate exista deja.\n");
  }
  char cale_fisier[256];
  sprintf(cale_fisier, "%s/reports.dat", nume_director);
  int fd_reports=open(cale_fisier, O_CREAT | O_RDWR | O_APPEND, 0664);
  if(fd_reports>=0){
    close(fd_reports);
  }
  sprintf(cale_fisier, "%s/district.cfg", nume_director);
  int fd_cfg=open(cale_fisier, O_CREAT | O_RDWR, 0640);
  if(fd_cfg>=0){
    close(fd_cfg);
  }
  sprintf(cale_fisier, "%s/logged_district", nume_director);
  int fd_logged=open(cale_fisier, O_CREAT | O_RDWR | O_APPEND, 0644);
  if(fd_logged>=0){
    close(fd_logged);
  }
}
//functie pt creat fisiere
/*creat(const char *name, mode_t mode){
  }*/
void set_mode(mode_t mod, char permisiuni[10]){
  int i;
  for( i=0;i<=8;i++){
    permisiuni[i]='-';
  }
  if(mod & S_IRUSR){
    permisiuni[0]='r';
  }
  if(mod & S_IWUSR){
    permisiuni[1]='w';
  }
  if(mod & S_IXUSR){
    permisiuni[2]='x';
  }
  if(mod & S_IRGRP){
    permisiuni[3]='r';
  }
  if(mod & S_IWGRP){
    permisiuni[4]='w';
  }
  if(mod & S_IXGRP){
    permisiuni[5]='x';
  }
  if(mod & S_IROTH){
    permisiuni[6]='r';
  }
  if(mod & S_IWOTH){
    permisiuni[7]='w';
  }
  if(mod & S_IXOTH){
    permisiuni[8]='x';
  }
  permisiuni[9]='\0';
}
int main(int argc, const char** argv){
  /*if(argc>5){
    perror("Prea multe argumente!\n");
    exit(1);
  }
  Report report;
  add <district_id>
  list <district_id>
  */
  //create_director("district1");
  //functii cu ai
  int parse_condition(const char* input, char* field, char* op, char* value);
  int match_condition(Report* r, const char* field, const char* op, const char* value);
  return 0;
}
