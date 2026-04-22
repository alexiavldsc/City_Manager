#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
typedef struct{
  float latitude;
  float longitude;
}Coordinates;
enum Security_level{
  minor,
  moderate,
  critical
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
/*void create_director(char director_name[15]){
  mkdir(director_name, "750");
  }*/
//functie pt creat fisiere
/*creat(const char *name, mode_t mode){
  }*/
void set_mode(mode_t mod, char permisiuni[10]){
  int i;
  for( i=0;i<=8;i++){
    permisiuni[i]='-';
  }
  if(permisiuni[0] | S_IRUSR){
    permisiuni[0]='r';
  }
  if(permisiuni[1] | S_IWUSR){
    permisiuni[1]='w';
  }
  if(permisiuni[2] | S_IXUSR){
    permisiuni[2]='x';
  }
  if(permisiuni[3] | S_IRGRP){
    permisiuni[3]='r';
  }
  if(permisiuni[4] | S_IWGRP){
    permisiuni[4]='w';
  }
  if(permisiuni[5] | S_IXGRP){
    permisiuni[5]='x';
  }
  if(permisiuni[6] | S_IROTH){
    permisiuni[6]='r';
  }
  if(permisiuni[7] | S_IWOTH){
    permisiuni[7]='w';
  }
  if(permisiuni[8] | S_IXOTH){
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
