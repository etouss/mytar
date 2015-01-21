#include <time.h>
#include "struct.h"

/**
Fonctione prenant l'index des fichier en parametrant et libérant la mémoire.
*/
void free_index(Fichier *index) {
    int i = 0;
    while (42) {
        if (index[i].path[0] == 0) break;
        free(index[i].path);
        free(index[i].real_path);
        i++;
    }
    free(index);
}

void free_index2(Fichier *index) {
  int i = 0;
  while (42) {
    if (index[i].path == NULL) break;
    free(index[i].path);
    free(index[i].real_path);
    i++;
  }
  free(index);
}

void errxFree(Fichier *toFree1,void *toFree2,void *toFree3, const char* format, ...){
  if(toFree1 != NULL)free_index2(toFree1);
  if(toFree2 != NULL)free(toFree2);
  if(toFree3 != NULL)free(toFree3);
  va_list a_list;
  errx(1,format,a_list);
}

/**
Fonction permetant de récuperer le nom d'un fichier (en enlevant son chemin)
Et le stock dans name
*/
void get_file_name(char *path, char **name) {
    *name = path;
    //printf("%s\n",name);
    int i = 0;
    int j = 0;
    while (path[i] != 0) {
        if (path[i] == '/')j = i;
        i++;
    }
    *name += j;
    if (j != 0)*name += 1;
    //printf("%s\n",name);
}
/**
Fonction prenant en parametre l'index de description d'un fichier, verfie si elle
peut mettre le type de lock demander et renvoie le retour le cas échéant.
*/
/*int lockfile(int fd, int type) {
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
    //printf("HEY\n");
    return -1;
}
*/

/**
Fonction convertisant un int i en string de 10 caractere.
*/
char *recup(int i) {
    char *buffer = calloc(10, sizeof(char));
    if(buffer == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    char *result = calloc(10, sizeof(char));
    if(result == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    int j = 10;
    int c = 0;
    while (i != 0) {
        buffer[c] = (i % j) + '0';
        i /= 10;
        c++;
    }
    for (j = 0; j < c; j++) {
        result[j] = buffer[(c - 1) - j];
    }
    return result;
}

/**
Fonction prenant un mode_t en renvoyant une string décrivant ces droit est type pour ls
*/
char *info(mode_t mode) {
    char type = '-';
    char *rwx = malloc(sizeof(char) * 10);
    if(rwx == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    int i;


    if (S_ISDIR(mode)) {
        type = 'd';
    }
    else if (S_ISLNK(mode)) {
        type = 'l';
    }
    else if (S_ISFIFO(mode)) {
        type = 'f';
    }
    else if (S_ISBLK(mode)) {
        type = 'b';
    }
    else if (S_ISCHR(mode)) {
        type = 'c';
    }

    for (i = 0; i < 9; i++) {
        rwx[i] = '-';
    }

    if (mode & S_IRUSR) {
        rwx[0] = 'r';
    }

    if (mode & S_IWUSR) {
        rwx[1] = 'w';
    }
    if (mode & S_IXUSR) {
        rwx[2] = 'x';
    }
    if (mode & S_IRGRP) {
        rwx[3] = 'r';
    }
    if (mode & S_IWGRP) {
        rwx[4] = 'w';
    }
    if (mode & S_IXGRP) {
        rwx[5] = 'x';
    }
    if (mode & S_IROTH) {
        rwx[6] = 'r';
    }
    if (mode & S_IWOTH) {
        rwx[7] = 'w';
    }
    if (mode & S_IXOTH) {
        rwx[8] = 'x';
    }
    rwx[9] = 0;
    char *final = malloc(sizeof(char) * (sizeof(char) + strlen(rwx) + 2));
    if(final == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    sprintf(final, "%c%s", type, rwx);
    free(rwx);

    //printf("%s\n", final);
    return final;
}

/**
Fonction vérifiant si les fichier de l'archive et ceux passer en parametre
correspondent, pour savoir si une action doit être effectuer.
non Contraignante sur les parametre.
*/
boolean show(char **file, int size, char *path) {
    if (size == 0) return true;
    int i;
    for (i = 0; i < size; i++) {
        if (strncmp(path, file[i], strlen(file[i])) == 0)return true;
        //if(strcmp(file[i], path) == 0) return true
    }
    return false;
}

/**
Fonction vérifiant si les fichier de l'archive et ceux passer en parametre
correspondent, pour savoir si une action doit être effectuer.
Contraignante sur les parametre.
*/
boolean show2(char ** file, int size, char * path){
  if (size == 0) return true;
  int i;
  for(i = 0; i<size; i++){
    if(strncmp(path, file[i],strlen(path)) == 0)return true;
    //if(strcmp(file[i], path) == 0) return true
  }
  return false;
}

/**
Fonction qui list les element d'une archive donnée en parametre.
Au format ls -l partiel
*/
boolean list_archive(Fichier * index, char ** file, int size){
    int i=0;
    while(42) {
        if(index[i].path[0]==0) break;
        if(!show(file, size, index[i].path)){
            i++;
            continue;
        }
        struct tm *timeb;
        char *droit = info(index[i].entete.mode);
        timeb = localtime(&(index[i].entete.m_time));


        printf("%s %10ld %2d %2d %2d:%2d", droit, index[i].entete.file_length, timeb->tm_mday, timeb->tm_mon, timeb->tm_hour, timeb->tm_min);
        if (droit[0] == 'd') {
            //printf("\e[0;92m %s\e[0m\n", index[i].path);
            printf(" %s\n", index[i].path);
        }
        else if (droit[0] == 'l') {
            //printf("\e[0;95m %s\e[0m\n", index[i].path);
            printf(" %s\n", index[i].path);
        }
        else {
            printf(" %s\n", index[i].path);
        }
        free(droit);
        i++;
    }
    return false;

}


/**
Fonction verifiant si le path donnée fini par mtr ou mtr.gz
*/
int endsWith_mtr_or_mtrgz(char *src) {
    int mtr = true;
    if (strlen(src) <= 4) return 0;
    mtr = mtr && src[strlen(src) - 1] == 'r';
    mtr = mtr && src[strlen(src) - 2] == 't';
    mtr = mtr && src[strlen(src) - 3] == 'm';
    mtr = mtr && src[strlen(src) - 4] == '.';
    if (mtr) return 1;
    mtr = true;
    if (strlen(src) <= 7) return 0;
    mtr = mtr && src[strlen(src) - 1] == 'z';
    mtr = mtr && src[strlen(src) - 2] == 'g';
    mtr = mtr && src[strlen(src) - 3] == '.';
    mtr = mtr && src[strlen(src) - 4] == 'r';
    mtr = mtr && src[strlen(src) - 5] == 't';
    mtr = mtr && src[strlen(src) - 6] == 'm';
    mtr = mtr && src[strlen(src) - 7] == '.';
    if (mtr) return 2;
    return 0;


}
