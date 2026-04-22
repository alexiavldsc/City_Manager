#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
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
//mkdir (District1);
//mkdir District2;
int main(int argc, const char** argv){
  /*if(argc>5){
    perror("Prea multe argumente!\n");
    exit(1);
  }
  if(argc<4){
    perror("Nu sunt suficiente argumente!\n");
    exit(1);
    }
    Report report;*/
  int parse_condition(const char* input, char* field, char* op, char* value);
  int match_condition(Report* r, const char* field, const char* op, const char* value);
  return 0;
}
