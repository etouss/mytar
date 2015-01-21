#include "lock.h"
// Here we go;
// test du lock si libre :
//      pose de verrou et go on:
// Si non
//      warning qu'il faut liberer le fichier
//      mise de l'index dans la pile/fifo ?

//Fin de l'archive des fichier libre
//Dépile en SetLKW avec retourn d'errno
//     Si erno : warn pas pu archiver le fichier
// else on continue .

//Structure necessaire : Fifo/Lifo list d'element a archiver.

//Fonction necessaire :
//         Test de lockage + lockage / mise dans fifo. (non bloquant)  == setLK()
//         Parcours de la lifo pour les dernier / (Bloquant) == forceLK()

//Moment d'appelle :
//  setLK() comme avant.
//  forceLK() aprés la boucle. sur index. on vide la pile.

// estimation : 5 heure de code ...
// estimation test : 2 heure.

/**
* Handler de SIGALRM vide
*/
void termination_handler (int signum){}


/**
* Fonction servant à la liste chainé.
*/
typedef struct Lc{
  Fichier f;
  struct Lc * next;
}Lc;

Lc* listeDebut;
Lc* listeFin;

void createElementLc(Fichier f, Lc * l){
  l->f = f;
  l->next = NULL;
}

void addLc(Lc * l){
  static int nb = 0;
  if(nb == 0){
    listeDebut = l;
    listeFin = l;
  }
  else{
    listeFin->next = l;
    listeFin = l;
  }
  nb++;
}

Fichier popLc(){
  //breakpoint();
  Fichier tmp;
  tmp = listeDebut->f;
  Lc * tmpLc = listeDebut->next;
  free(listeDebut);
  listeDebut = tmpLc;
  return tmp;
}

boolean isEmptyLc(){
  return listeDebut == NULL || listeDebut == 0;
}





typedef struct LcP{
  char * p;
  struct LcP * next;
}LcP;

/**
* Variable contenant le premier et dernier element de la liste chainé
* La dite liste stock les élément dont le verrou n'est pas passé.
*/
LcP* listeDebutP;
LcP* listeFinP;


void createElementLcP(char * p, LcP * l){
  l->p = p;
  l->next = NULL;
}

void addLcP(LcP * l){
  static int nb = 0;
  if(nb == 0){
    listeDebutP = l;
    listeFinP = l;
  }
  else{
    listeFinP->next = l;
    listeFinP = l;
  }
  nb++;
}

char * popLcP(){
  //breakpoint();
  char * tmp;
  tmp = listeDebutP->p;
  LcP * tmpLcP = listeDebutP->next;
  free(listeDebutP);
  listeDebutP = tmpLcP;
  return tmp;
}

boolean isEmptyLcP(){
  return listeDebutP == NULL || listeDebutP == 0;
}




int lockfileP(int fd, int type, char *p) {
  struct flock fl;
  fl.l_type = type;
  fl.l_start = 0;
  fl.l_whence = SEEK_SET;
  fl.l_len = 0;
  fcntl(fd, F_GETLK, &fl);
  if (fl.l_type == F_UNLCK) {
    fl.l_type = type;
    return fcntl(fd, F_SETLK, &fl);
  }
  else if(p == NULL)return -1;
  else{
    close(fd);
    struct LcP *l = malloc(sizeof(LcP));
    createElementLcP(p,l);
    addLcP(l);
    //breakpoint();
    return -1;
  }
}


/**
* Pose une verrou bloquant sur le fichier intérrompu aprés 10 sec.
*/
int forcelockfile(int fd,int type){
  int p = getpid();
  int f;
  if((f = fork())==0){
    sleep(10);
    kill(p,SIGALRM);
    exit(1);
  }
  //alarm(5);
  struct sigaction new_action;
  new_action.sa_handler = termination_handler;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;
  sigaction (SIGALRM, &new_action, NULL);

  struct flock fl;
  fl.l_type = type;
  fl.l_start = 0;
  fl.l_whence = SEEK_SET;
  fl.l_len = 0;
  //printf("Vous avez 10 sec pour lib\n");
  if(fcntl(fd, F_SETLKW, &fl) == -1){
    wait(NULL);
    errno = 0;
    return -1;
  }
  kill(f,SIGTERM);
  wait(NULL);
  errno = 0;
  return 0;
}

/**
* Pose un verrou non bloquant sur le fichier en cas d'échec ajoute ce dernier a la liste.
*/
int lockfile(int fd, int type, Fichier *f) {
  struct flock fl;
  fl.l_type = type;
  fl.l_start = 0;
  fl.l_whence = SEEK_SET;
  fl.l_len = 0;
  fcntl(fd, F_GETLK, &fl);
  if (fl.l_type == F_UNLCK) {
    fl.l_type = type;
    return fcntl(fd, F_SETLK, &fl);
  }
  else if(f == NULL)return -1;
  else{
    close(fd);
    struct Lc *l = malloc(sizeof(Lc));
    createElementLc(*f,l);
    addLc(l);
    //breakpoint();
    return -1;
  }
}
