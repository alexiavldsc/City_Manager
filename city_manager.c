#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
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
int verify_permissions(const char* cale_fisier, const char* role, int read, int write){
  struct stat status;
  if(stat(cale_fisier, &status)<0){
    perror("Eroare status fisier");
    exit(1);
  }
  if(strcmp(role, "manager")==0){ //testam permisiunile de user
    if(read && !(status.st_mode & S_IRUSR)){
      return 0;
    }
    if(write && !(status.st_mode & S_IWUSR)){
      return 0;
    }
  }
  else if(strcmp(role, "inspector")==0){ //testam permisiunile de group
    if(read && !(status.st_mode & S_IRGRP)){
      return 0;
    }
    if(write && !(status.st_mode & S_IWGRP)){
      return 0;
    }
  }
  else{
    return 0; //rol invalid
  }
  return 1;
}
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
void create_symlink(const char* nume_district){
  char nume_link[256];
  char target[256];
  sprintf(nume_link, "active_reports-%s", nume_district);
  sprintf(target, "%s/reports.dat", nume_district);
  unlink(nume_link);
  if(symlink(target, nume_link)<0){
    perror("Eroare la crearea de symlink");
    exit(1);
  }
}
void check_symlink(const char* nume_district){
  char nume_link[256];
  sprintf(nume_link, "active_reports-%s", nume_district);
  struct stat link_status;
  if(lstat(nume_link, &link_status)<0){
    return; //nu exista inca
  }
  if(!S_ISLNK(link_status.st_mode)){
    printf("'%s' nu este un symlink!\n", nume_link);
    return;
  }
  //verificam daca tinta exista
  struct stat target_status;
  if(stat(nume_link, &target_status)<0){
    printf("Symlink-ul '%s' este dangling (tinta nu exista)!\n", nume_link);
  }
}
void log_action(const char* district_id, const char* role, const char* user, const char* action){
  char cale_fisier[256];
  sprintf(cale_fisier, "%s/logged_district", district_id);
  //struct stat status;
  if(!verify_permissions(cale_fisier, role, 0, 1)){
    printf("Atentie! Rolul '%s' nu are permisiune de scriere pe %s. Actiunea nu va fi logata.\n", role, cale_fisier);
    return; //inspectorul isi poate face operatia
  }
  int fd=open(cale_fisier, O_WRONLY | O_APPEND | O_CREAT, 0644);
  if(fd<0){
    perror("Eroare la deschiderea fisierului logged_district");
    return;
  }
  time_t timp_curent=time(NULL);
  char line[512];
  int len=snprintf(line, sizeof(line), "%ld\t%s\t%s\t%s\n",(long)timp_curent, user, role, action);
  write(fd, line, len);
  close(fd);
}
void create_director(const char* nume_director){
  if(mkdir(nume_director, 0750)<0){
    if(errno!=EEXIST){
      perror("Eroare la crearea directorului");
      exit(1);
    }
  }
  chmod(nume_director,0750);
  char cale_fisier[256];
  sprintf(cale_fisier, "%s/reports.dat", nume_director);
  int fd_reports=open(cale_fisier, O_CREAT | O_RDWR | O_APPEND, 0664);
  if(fd_reports>=0){
    close(fd_reports);
    chmod(cale_fisier,0664);
  }else{
    perror("Eroare la crearea fisierului reports.dat");
    exit(1);
  }
  sprintf(cale_fisier, "%s/district.cfg", nume_director);
  int fd_cfg=open(cale_fisier, O_CREAT | O_RDWR, 0640);
  if(fd_cfg>=0){
    close(fd_cfg);
    chmod(cale_fisier,0640);
  }
  else{
    perror("Eroare la crearea fisierului district.cfg");
    exit(1);
  }
  sprintf(cale_fisier, "%s/logged_district", nume_director);
  int fd_logged=open(cale_fisier, O_CREAT | O_RDWR | O_APPEND, 0644);
  if(fd_logged>=0){
    close(fd_logged);
    chmod(cale_fisier,0644);
  }
  else{
    perror("Eroare la crearea fisierului logged_district");
    exit(1);
  }
}
//functii cu AI
int parse_condition(const char* input, char* field, char* op, char* value){
  if(strlen(input)>=256){ //sa nu depasim dim buffer-ului
    printf("Eroare! Prea lungi conditiile filtrului.\n");
    return 0;
  }
  //copiem inputul ca sa nu modificam originalul
  char buf[256];
  strncpy(buf, input, 255);
  buf[255] = '\0';
 
  char* first_colon = strchr(buf, ':');
  if (!first_colon) return 0;
  *first_colon = '\0';
  strcpy(field, buf);
 
  char* rest = first_colon + 1;
  char* second_colon = strchr(rest, ':');
  if (!second_colon) return 0;
  *second_colon = '\0';
  strcpy(op, rest);
 
  strcpy(value, second_colon + 1);
  return 1;
}
int match_condition(Report* r, const char* field, const char* op, const char* value){
  if (strcmp(field, "severity") == 0) {
    int val = atoi(value);
    int sev = (int)r->level;
    if (strcmp(op, "==") == 0) return sev == val;
    if (strcmp(op, "!=") == 0) return sev != val;
    if (strcmp(op, "<")  == 0) return sev <  val;
    if (strcmp(op, "<=") == 0) return sev <= val;
    if (strcmp(op, ">")  == 0) return sev >  val;
    if (strcmp(op, ">=") == 0) return sev >= val;
  } else if (strcmp(field, "category") == 0) {
    int cmp = strcmp(r->category, value);
    if (strcmp(op, "==") == 0) return cmp == 0;
    if (strcmp(op, "!=") == 0) return cmp != 0;
  } else if (strcmp(field, "inspector") == 0) {
    int cmp = strcmp(r->inspector, value);
    if (strcmp(op, "==") == 0) return cmp == 0;
    if (strcmp(op, "!=") == 0) return cmp != 0;
  } else if (strcmp(field, "timestamp") == 0) {
    time_t val = (time_t)atol(value);
    if (strcmp(op, "==") == 0) return r->timestamp == val;
    if (strcmp(op, "!=") == 0) return r->timestamp != val;
    if (strcmp(op, "<")  == 0) return r->timestamp <  val;
    if (strcmp(op, "<=") == 0) return r->timestamp <= val;
    if (strcmp(op, ">")  == 0) return r->timestamp >  val;
    if (strcmp(op, ">=") == 0) return r->timestamp >= val;
  }
  else{
    printf("Campul '%s' nu reprezinta un criteriu de filtrare.\n", field);
  }
  return 0;
}
void add(const char* district_id, const char* role, const char* user){
  char cale_fisier[256];
  sprintf(cale_fisier, "%s/reports.dat", district_id);
  struct stat status;
  if(stat(district_id, &status)!=0){
    create_director(district_id);
  }
  create_symlink(district_id);
  check_symlink(district_id);
  if(!verify_permissions(cale_fisier, role, 0, 1)){
    printf("Eroare! Rolul '%s' nu are permisiuni de scriere pentru %s.\n", role, cale_fisier);
    return;
  }
  chmod(cale_fisier, 0664);
  int fd=open(cale_fisier, O_RDWR | O_APPEND);
  if(fd<0){
    perror("Eroare la deschiderea reports.dat");
    return;
  }
  //luam id-ul ultimului raport +1 si il consideram id-ul nou
  struct stat fisier_status;
  fstat(fd, &fisier_status);
  int id_nou=1; //in caz ca e gol
  if(fisier_status.st_size>0){
    Report prev_report;
    lseek(fd, -sizeof(Report), SEEK_END);
    read(fd, &prev_report, sizeof(Report));
    id_nou=prev_report.id+1;
  }
  Report report;
  report.id=id_nou;
  strncpy(report.inspector, user, sizeof(report.inspector)-1);
  report.inspector[sizeof(report.inspector)-1]='\0';
  report.timestamp=time(NULL);
  printf("X: ");
  scanf("%f", &report.coordinates.latitude);
  printf("Y: ");
  scanf("%f", &report.coordinates.longitude);
  printf("Category (road/lighting/flooding/etc): ");
  scanf("%9s", report.category);
  int severity;
  printf("Severity level (1/2/3): ");
  scanf("%d", &severity);
  report.level=(enum Severity_level)severity;
  int c;
  while((c=getchar())!='\n' && c!=EOF);
  printf("Description: ");
  fgets(report.description, sizeof(report.description), stdin);
  report.description[strcspn(report.description, "\n")]=0; //eliminăm \n de la final
  lseek(fd, 0, SEEK_END); //scriem la sfarsit
  if(write(fd, &report, sizeof(Report)) != sizeof(Report)){
    perror("Eroare la scrierea raportului");
    return;
  }
  else{
    printf("Raport adaugat! ID: %d\n", id_nou);
  }
  close(fd);
}
void list(const char* district_id, const char* role){
  char cale_fisier[256];
  sprintf(cale_fisier, "%s/reports.dat", district_id);
  if(!verify_permissions(cale_fisier, role, 1, 0)){
    printf("Eroare! Rolul '%s' nu are permisiuni de citire pentru %s.\n", role, cale_fisier);
    return; //abandoneaza operatia
  }
  struct stat fisier_status; //pt statusul fisierului
  if(stat(cale_fisier, &fisier_status)<0){
    perror("Eroare la stat() pt reports.dat");
    return;
  }
  char permisiuni[10];
  set_mode(fisier_status.st_mode, permisiuni);
  char time_str[64];
  strncpy(time_str, ctime(&fisier_status.st_mtime), sizeof(time_str));
  time_str[strcspn(time_str, "\n")]=0; //eliminam \n
  printf("Fisier: %s | Permisiuni: %s | Dimensiune: %ld bytes | Modificat: %s\n", cale_fisier, permisiuni, (long)fisier_status.st_size, time_str);
  printf("\n");
  int fd=open(cale_fisier, O_RDONLY);
  if(fd<0){
    perror("Nu s-a putut deschide reports.dat");
    return;
  }
  Report report;
  while(read(fd, &report, sizeof(Report)) == sizeof(Report)){
    printf("ID: %d | Inspector: %s | Categorie: %s | Severitate: %d\n", report.id, report.inspector, report.category, (int)report.level);
  }
  close(fd);
}
void view(const char* district_id, const char* role, int target_id){
  char cale_fisier[256];
  sprintf(cale_fisier, "%s/reports.dat", district_id);
  if(!verify_permissions(cale_fisier, role, 1, 0)){
    printf("Eroare! Rolul '%s' nu are permisiuni de citire pentru %s.\n", role, cale_fisier);
    return;
  }
  int fd=open(cale_fisier, O_RDONLY);
  if(fd<0){
    perror("Nu s-a putut deschide reports.dat");
    return;
  }
  Report report;
  int gasit=0;
  while(read(fd, &report, sizeof(Report)) == sizeof(Report)){
    if(report.id == target_id){
      gasit=1;
      char time_str[64];
      strncpy(time_str, ctime(&report.timestamp), sizeof(time_str));
      time_str[strcspn(time_str, "\n")] = 0;
      printf("Detalii Raport %d\n", report.id);
      printf("Inspector: %s\n", report.inspector);
      printf("Coordonate: X=%.2f, Y=%.2f\n", report.coordinates.latitude, report.coordinates.longitude);
      printf("Categorie: %s\n", report.category);
      printf("Severitate: %d\n", (int)report.level);
      printf("Timestamp: %s\n", time_str);
      printf("Descriere: %s\n", report.description);
      break;
    }
  }
  if(!gasit){
    printf("Raportul cu ID-ul %d nu a fost gasit in %s.\n", target_id, district_id);
  }
  close(fd);
}
void remove_report(const char* district_id, const char* role, int target_id){
  char cale_fisier[256];
  sprintf(cale_fisier, "%s/reports.dat", district_id);
  if(strcmp(role, "manager")!=0){
    printf("Eroare! Doar rolul 'manager' poate sterge rapoarte.\n");
    return;
  }
  if(!verify_permissions(cale_fisier, role, 1, 1)){
    printf("Eroare! Rolul '%s' nu are permisiuni rw pe %s.\n", role, cale_fisier);
    return;
  }
  int fd=open(cale_fisier, O_RDWR);
  if(fd<0){
    perror("Nu s-a putut deschide reports.dat pentru stergere");
    return;
  }
  struct stat status_inainte; //memoram dimensiunea initiala
  fstat(fd, &status_inainte);
  Report report;
  int gasit=0;
  off_t target_pos=0; //retinem la ce byte gasim raportul
  while(read(fd, &report, sizeof(Report)) == sizeof(Report)){
    if(report.id == target_id){
      gasit=1; //l-am gasit, suntem cu o pozitie dupa
      target_pos=lseek(fd, -sizeof(Report), SEEK_CUR); //ne mutam cu o pozitie inapoi
      break;
    }
  }
  if(!gasit){
    printf("Raportul cu ID-ul %d nu a fost gasit.\n", target_id);
    close(fd);
    return;
  }
  //shiftam rapoartele
  off_t read_pos=target_pos+sizeof(Report); //de unde citim (de după cel sters)
  off_t write_pos=target_pos; //unde scriem (peste raportul sters)
  lseek(fd, read_pos, SEEK_SET); //citim urmatorul
  while(read(fd, &report, sizeof(Report)) == sizeof(Report)){
    read_pos=lseek(fd, 0, SEEK_CUR); //salvam pozitia
    lseek(fd, write_pos, SEEK_SET);
    write(fd, &report, sizeof(Report));
    write_pos=lseek(fd, 0, SEEK_CUR); 
    lseek(fd, read_pos, SEEK_SET); //ne intoarcem pt urm bucla
  }
  if(ftruncate(fd, status_inainte.st_size-sizeof(Report))<0){ //taiem ultimul report ramas in plus la final
    perror("Eroare la ftruncate");
    return;
  }
  else{
    printf("Raportul %d a fost sters.\n", target_id);
  }
  struct stat status_dupa;
  fstat(fd, &status_dupa);
  printf("Dimensiune inainte: %ld, Dimensiune dupa: %ld\n", (long)status_inainte.st_size, (long)status_dupa.st_size);
  close(fd);
}
void update_threshold(const char* district_id, const char* role, int value){
  if(strcmp(role, "manager")!=0){
    printf("Eroare! Doar rolul 'manager' poate actualiza threshold-ul.\n");
    return;
  }
  char cale_fisier[256];
  sprintf(cale_fisier, "%s/district.cfg", district_id);
  struct stat status;
  if(stat(cale_fisier, &status)<0){
    perror("Eroare la verificarea fisierului district.cfg");
    return;
  }
  if((status.st_mode&0777)!=0640){ //pt a izola ult 3 cifre octale + verificat daca permisiunile sunt 640
    printf("Eroare! Permisiunile fisierului %s nu sunt 640.\n", cale_fisier);
    return;
  }
  int fd=open(cale_fisier, O_WRONLY | O_TRUNC);
  if(fd<0){
    perror("Nu s-a putut deschide district.cfg pentru scriere");
    return;
  }
  char buffer[32];
  int len=sprintf(buffer, "%d\n", value);
  if(write(fd, buffer, len)!=len){
    perror("Eroare la scrierea noului threshold");
    return;
  }
  else{
    printf("Threshold actualizat la %d pentru districtul '%s'.\n", value, district_id);
  }
  close(fd);
}
void filter(const char* district_id, const char* role, const char** conditions, int nr_conditions){
  char cale_fisier[256];
  sprintf(cale_fisier, "%s/reports.dat", district_id);
  if(!verify_permissions(cale_fisier, role, 1, 0)){ //pt citire
    printf("Eroare! Rolul '%s' nu are permisiuni de citire pentru filtrare.\n", role);
    return;
  }
  int fd=open(cale_fisier, O_RDONLY);
  if(fd<0){
    perror("Eroare la deschiderea reports.dat pentru filtrare");
    return;
  }
  Report report;
  int gasit=0;
  while(read(fd, &report, sizeof(Report)) == sizeof(Report)){
    int verifica_conditiile=1;
    for(int i=0;i<nr_conditions;i++){
      char field[32], op[8], value_str[64];
      if(!parse_condition(conditions[i], field, op, value_str)){
        verifica_conditiile=0;
        break;
      }
      if(!match_condition(&report, field, op, value_str)){
        verifica_conditiile = 0;
        break;
      }
    }
    if(verifica_conditiile){
      gasit++;
      printf("ID: %d | Inspector: %s | Categorie: %s | Severitate: %d\n", report.id, report.inspector, report.category, (int)report.level);
    }
  }
  if(gasit == 0){
    printf("Niciun raport nu a corespuns criteriilor de filtrare.\n");
  }else{
    printf("\nTotal rapoarte gasite: %d\n", gasit);
  }
  close(fd);
}
void check_all_symlinks() {
  DIR *dir;
  struct dirent *entry;
  struct stat link_status, target_status;
  dir=opendir(".");
  if(dir==NULL){
    perror("Eroare la scanarea directorului curent");
    return; 
  }
  while((entry = readdir(dir))!=NULL){
    if(strncmp(entry->d_name, "active_reports-", 15) == 0){
      if(lstat(entry->d_name, &link_status) == 0){
	if(S_ISLNK(link_status.st_mode)){
	  if(stat(entry->d_name, &target_status)<0){
	    printf("Symlink-ul '%s' este dangling (tinta nu exista)!\n", entry->d_name);
	  }
	}
      }
    }
  }
  closedir(dir);
}
void remove_district(const char* district_id, const char* role, const char* user){
  if(strcmp(role, "manager") != 0){
    printf("Rolul '%s' nu are voie sa apeleze comanda asta! Doar manager-ul poate!\n", role);
    exit(1);
  }
  if(strchr(district_id,'/') != NULL || strstr(district_id, "..") != NULL || strlen(district_id) == 0 || strcmp(district_id, ".") == 0){
    printf("Nume de district invalid (%s) !\n", district_id);
    exit(1);
  }
  struct stat st;
  if(stat(district_id, &st)<0){
    printf("Districtul '%s' nu exista!\n", district_id);
    return;
  }
  log_action(district_id, role, user, "remove_district");
  pid_t pid=fork();
  if(pid<0){
    perror("Eroare la fork");
    exit(1);
  }
  if(pid == 0){ //copil
    execlp("rm", "rm", "-rf", district_id, NULL);
    perror("Eroare la exec rm");
    exit(1);
  }
  else{
    //parinte
    int status;
    waitpid(pid, &status, 0);
    if(!WIFEXITED(status) || WEXITSTATUS(status) != 0){
      printf("Eroare: rm -rf nu s-a executat corect!\n");
      return;
    }
  }
  char nume_symlink[256];
  snprintf(nume_symlink, sizeof(nume_symlink), "active_reports-%s", district_id);
  if(unlink(nume_symlink) == 0){
    printf("Symlink-ul '%s' a fost sters!\n", nume_symlink);
  }
  else{
    perror("Eroare la stergerea symlink-ului");
  }
}
int main(int argc, const char** argv){
  const char* role=NULL;
  const char* user=NULL;
  const char* action=NULL;
  const char* district_id=NULL;
  int report_id=-1;
  int value=-1;
  const char* conditions[35];
  int nr_conditions=0;
  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"--role")==0 && i+1<argc){
      role=argv[++i];
    }
    else if(strcmp(argv[i],"--user")==0 && i+1<argc){
      user=argv[++i];
    }
    else if(strcmp(argv[i],"--add")==0 && i+1<argc){
      action="add";
      district_id=argv[++i];
    }
    else if(strcmp(argv[i],"--list")==0 && i+1<argc){
      action="list";
      district_id=argv[++i];
    }
    else if(strcmp(argv[i],"--view")==0 && i+2<argc){
      action="view";
      district_id=argv[++i];
      report_id=atoi(argv[++i]);
    }
    else if(strcmp(argv[i],"--remove_report")==0 && i+2<argc){
      action="remove_report";
      district_id=argv[++i];
      report_id=atoi(argv[++i]);
    }
    else if(strcmp(argv[i],"--update_threshold")==0 && i+2<argc){
      action="update_threshold";
      district_id=argv[++i];
      value=atoi(argv[++i]);
    }
    else if(strcmp(argv[i],"--filter")==0 && i+1<argc){
      action="filter";
      district_id=argv[++i];
      while(i+1<argc && argv[i+1][0]!='-'){
	conditions[nr_conditions++]=argv[++i];
      }
    }
    else if(strcmp(argv[i],"--remove_district")==0 && i+1<argc){
      action="remove_district";
      district_id=argv[++i];
    }
  }
  if(!role || !user || !action || !district_id){
    printf("Comanda incompleta!\n");
    exit(1);
  }
  check_all_symlinks();
  if(strcmp(action, "list") == 0){
    list(district_id, role);
  }
  else if(strcmp(action, "view") == 0){
    if(report_id == -1){
      printf("Eroare. Lipseste ID-ul raportului.\n");
      exit(1);
    }
    view(district_id, role, report_id);
  }
  else if(strcmp(action, "add") == 0){
    add(district_id, role, user);
  }
  else if(strcmp(action, "remove_report") == 0){
    if(report_id == -1){
      printf("Eroare! Lipseste ID-ul raportului.\n");
      exit(1);
    }
    remove_report(district_id, role, report_id);
  }
  else if(strcmp(action, "update_threshold") == 0){
    if(value == -1){
      printf("Eroare! Lipseste valoarea pt threshold.\n");
      exit(1);
    }
    update_threshold(district_id, role, value);
  }
  else if(strcmp(action, "filter") == 0){
    if(nr_conditions == 0){
      printf("Eroare! Lipsesc conditiile pt filtrare (minim una).\n");
      exit(1);
    }
    filter(district_id, role, conditions, nr_conditions);
  }
  else if(strcmp(action, "remove_district") == 0){
    remove_district(district_id, role, user);
  }
  if(strcmp(action, "remove_district") != 0){
    log_action(district_id, role, user, action);
  }
  return 0;
}
