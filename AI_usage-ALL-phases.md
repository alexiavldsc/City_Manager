# Documentatie AI Usage

# Faza 1

## Ce tool am folosit
Gemini

## Functii generate cu inteligenta artificiala

### 1. parse_condition
**Prompt-ul dat:**
"Lucrez la un proiect in C despre infrastructura unui oras si am nevoie de o functie pentru parsarea unor filtre. Scrie o functie parse_condition(const char* input, char* field, char* op, char* value) care primeste un string de forma field:operator:value si il imparte in cele trei variabile. Conditii stricte: nu folosi strtok. Vreau sa copiezi input-ul intr-un buffer local de 256 caractere pentru a nu modifica sirul original, sa folosesti strchr si sa inlocuiesti caracterele ':' cu '\0'."

**Ce a fost generat:**
Gemini a generat o functie care foloseste strchr si pointeri pentru a inlocui ':' cu '\0' si extrage field, operator si value intr-un mod destructiv pe o copie a input-ului.

**Ce am schimbat:**
1. Am adaugat manual o verificare initiala pentru a preveni un potential buffer overflow inainte de a folosi functia de copiere strncpy.
2. Am adaugat buf[255]='\0' explicit pentru siguranta, garantand ca sirul copiat este null-terminated chiar daca input-ul are exact 255 de caractere.

**Ce am invatat:**
1. Cum functioneaza strchr() pentru a cauta caractere intr-un string si cum ajuta aritmetica de pointeri la partitionarea unui sir de caractere.
2. Faptul ca in terminalul de Linux trebuie pus un `'\'` inaintea operatorului '>'. Am invatat si in cadrul primelor laboratoare de SO ca acest backslash este procesat exclusiv de shell (pentru a evita redirectarea stdout-ului in fisiere).
---
### 2. match_condition

**Prompt-ul dat:**
"Avand urmatoarele structuri de date in C pentru proiectul meu (Coordinates, enum Severity_level si struct Report), am extras deja dintr-un string conditiile sub forma de 3 variabile: field, op (operatorul) si value. Scrie o functie match_condition(Report *r, const char *field, const char *op, const char *value) care verifica daca un pointer la structura Report respecta aceasta conditie. Returneaza 1 daca se potriveste si 0 daca nu."

**Ce a fost generat:**
Gemini a generat o functie bazata pe blocuri de tip if - else if, care compara un string cu numele campurilor din structura, transforma valorile extrase din format text in numere folosind atoi sau atol si evalueaza logic operatorul dat ca string.

**Ce am modificat:**
1. Am verificat manual si testat conversiile de tip (m-am asigurat ca enum Severity_level suporta cast la int pentru evaluare).
2. Am adaugat o ramura else la final pentru a afisa o atentionare in caz ca utilizatorul introduce gresit filtrul in linia de comanda, facand programul mai robust.

**Ce am invatat:**
1. Cum sa compar tipuri diferite (int, string, time_t) dintr-o functie generica
2. Importanta adaugarii unui mecanism de verificare in caz ca utilizatorul nu introduce argumentele corect sau nu sunt optiuni valabile.

# Faza 2

Pentru faza 2, am folosit AI-ul pentru a intelege concepte si functii noi de sistem. L-am folosit pentru:
1. Semnale: ca sa inteleg cum functioneaza structura sigaction pentru a prinde semnalele.
2. Procese: am avut nevoie de niste explicatii despre cum functioneaza sincronizarea dintre procesul parinte si copil la comanda rm -rf si cum sa verific daca stergerea s-a terminat cu succes folosind WIFEXITED.
3. Trimiterea de semnale: am folosit AI-ul pentru a intelege cum se foloseste functia kill() si cum trebuie tratate erorile in cazul in care semnalul nu poate fi trimis.

# Faza 3

Pentru Faza 3, am folosit AI-ul pentru a clarifica detalii tehnice legate de standardul POSIX. L-am folosit pentru:
1. Clarificarea diferentei dintre exit() si _exit(): am invatat ca exit() face "flush" la bufferele de I/O standard din C (cum ar fi stdout), ceea ce poate duce la afisari duplicate daca parintele avea date care nu au fost flush-uite inainte de fork() si ca mai bine as folosi _exit() pentru terminarea copiilor, care ocoleste curatarea bufferelor de la nivelul bibliotecii C.
2. Debugging pe pipe-uri si dup2(): am cerut explicatii despre cum sa previn blocajele atunci cand folosesc pipe() si dup2() pentru a redirecta output-ul din scorer catre city_hub. AI-ul m-a ajutat sa inteleg importanta inchiderii capetelor de scriere (pipes[i][1]) in procesul parinte inainte de a incerca sa citesc din ele, altfel apelul read() din parinte ar fi ramas blocat la nesfarsit asteptand date.
3. Gestionarea erorilor: am cerut sfaturi despre cum as putea face functia calculate_scores imuna la esecuri de sistem. Am invatat ca, daca fork() esueaza la un anumit district, programul nu trebuie sa se opreasca complet. Solutia pe care am integrat-o a fost marcarea esecului cu -1 in vectorul de PID-uri si trecerea la urmatorul district folosind continue, avand grija sa inchid capetele pipe-ului nefolosit pentru a preveni memory leaks.


