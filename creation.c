//
//  creation.c
//  myTar
//
//  Created by Etienne Toussaint on 30/11/2014.
//  Copyright (c) 2014 Etienne Toussaint. All rights reserved.
//

#include <fcntl.h>
#include "creation.h"


void creer_index_from_file(char * argv,char * path,struct stat st,Fichier *indexCourant, int args,char * repC, boolean first);

/**
Fonction vérifiant si le fichier en position indice dans l'index est
bien le fichier le plus recent.
*/
void last_modif(Fichier *index,int indice){
  int i = 0;
  for (i=0;i<indice;i++){
    if(strcmp(index[i].path,index[indice].path)==0){
      //breakpoint();
      if (index[i].entete.m_time  > index[indice].entete.m_time)index[indice].skip = true;
      else index[i].skip = true;
    }
  }
}

/**
Fonction créant l'index d'un dossier passer en parametre et de toute son arborencense.
*/
int creer_index_from_dir(char * real_path,char * path ,struct stat st,Fichier **index, int indice, int * MAX_FICHIER, int args, char * repC, boolean first){
  int nb_fichier_creers = 0;
  DIR *dir = NULL;
  if(args & REP && first) {
    char *tmp = NULL;
    //printf("1:%s\n",path);
    get_file_name(path, &tmp);
    //printf("2:%s\n",tmp); // getcwd()?? sinon . LOOPER
    (*index)[indice+nb_fichier_creers].path = malloc(sizeof(char)*(strlen(repC)+strlen(tmp)+2));
    if((*index)[indice+nb_fichier_creers].path == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    sprintf((*index)[indice+nb_fichier_creers].path, "%s/%s",repC,tmp);
    path = (*index)[indice+nb_fichier_creers].path;
    //printf("%s\n",(*index)[indice+nb_fichier_creers].path);
  }
  else{
    (*index)[indice+nb_fichier_creers].path = malloc(sizeof(char)*(strlen(path)+1));
    if((*index)[indice+nb_fichier_creers].path == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    sprintf((*index)[indice+nb_fichier_creers].path, "%s",path);
  }
  (*index)[indice+nb_fichier_creers].entete.path_length = strlen(path);
  (*index)[indice+nb_fichier_creers].entete.file_length = 0;
  (*index)[indice+nb_fichier_creers].entete.mode = st.st_mode;
  (*index)[indice+nb_fichier_creers].entete.m_time = st.st_mtime;

  (*index)[indice+nb_fichier_creers].archive = false;
  (*index)[indice+nb_fichier_creers].real_path = malloc(sizeof(char)*(strlen(real_path)+1));
  if((*index)[indice+nb_fichier_creers].real_path == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
  sprintf((*index)[indice+nb_fichier_creers].real_path,"%s",real_path);
  //indice+nb_fichier_creers ++;
  nb_fichier_creers ++;
  //printf("argv :%s\n",argv);
  dir = opendir(real_path);
  if(dir == NULL) {
  /*fprintf(stderr,"lsx: %s:acces impossible\n",argv);*/
    if(errno != 0)perror(strerror(errno));
    errno = 0;
    return nb_fichier_creers;
  }
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
      if(indice+nb_fichier_creers == *MAX_FICHIER) {
        *MAX_FICHIER += 100;
        *index = realloc(*index,sizeof(Fichier)*(*MAX_FICHIER));
        if(*index == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
      }
      if ((entry->d_name[0] == '.' && strlen(entry->d_name) == 1) ||
        (entry->d_name[0] == '.'&& entry->d_name[1] == '.'
        && strlen(entry->d_name) == 2)) {
          //printf("fegr");
          continue;
        }

        struct stat *tmp = malloc(sizeof(struct stat));
        if(tmp == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        char *real_chemin = malloc(sizeof(char)*(strlen(real_path)+strlen(entry->d_name)+2));
        if(real_chemin == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        sprintf(real_chemin, "%s/%s",real_path,entry->d_name);
        //printf("realChemin:%s \n",real_chemin);
        char *chemin = malloc(sizeof(char)*(strlen(path)+strlen(entry->d_name)+2));
        if(chemin == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        sprintf(chemin, "%s/%s",path,entry->d_name);
        //printf("chemin:%s \n",chemin);
        if(args & SIMLINK){
          lstat(real_chemin, tmp);
          //index[indice+nb_fichier_creers]->simlink = true;
        }
        else{
          stat(real_chemin,tmp);
          //index[indice+nb_fichier_creers]->simlink = false;
        }
        if(S_ISREG(tmp->st_mode)||S_ISLNK(tmp->st_mode)) {
          creer_index_from_file(real_chemin,chemin,*tmp,&((*index)[indice+nb_fichier_creers]),args,repC,false);
          if(args & (SKIP_OLD|ADD)){
            last_modif(*index,indice+nb_fichier_creers);
          }
          nb_fichier_creers++;
        }
        if(S_ISDIR(tmp->st_mode)) {
          //printf("%s\n",real_chemin);
          nb_fichier_creers += creer_index_from_dir(real_chemin,chemin,*tmp,index,indice+nb_fichier_creers,MAX_FICHIER,args,repC,false);
        }
        //Probable probleme de free
        free(tmp);
        free(real_chemin);
        free(chemin);
      }
      closedir(dir);
      return nb_fichier_creers;
    }

/**
Fonction créant l'index correspondant a un fichier.
*/
void creer_index_from_file(char * argv,char * path,struct stat st,Fichier *indexCourant, int args,char * repC, boolean first){
      if(args & REP && first) {
        char * tmp= NULL;
        //printf("1:%s\n",argv);
        get_file_name(argv, &tmp);
        //printf("2:%s\n",tmp);
        indexCourant->path = malloc(sizeof(char)*(strlen(repC)+strlen(tmp)+2));
        if(indexCourant->path == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        //printf("%s\n",argv);
        sprintf(indexCourant->path, "%s/%s",repC,tmp);
      }
      else if(args & REP){
        //char * tmp= NULL;
        //printf("1:%s\n",argv);
        //get_file_name(argv, &tmp);
        indexCourant->path = malloc(sizeof(char)*(strlen(path)+1));
        if(indexCourant->path == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        //printf("%s\n",argv);
        sprintf(indexCourant->path, "%s",path);
      }
      else{
        indexCourant->path = malloc(sizeof(char)*(strlen(argv)+1));
        if(indexCourant->path == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        sprintf(indexCourant->path, "%s",argv);
      }
      indexCourant->entete.path_length = strlen(indexCourant->path);
      indexCourant->entete.file_length = st.st_size;
      indexCourant->entete.mode = st.st_mode;
      indexCourant->entete.m_time = st.st_mtime;
      indexCourant->entete.checksum[0] = 0;
      indexCourant->archive = false;
      indexCourant->skip = false;
      indexCourant->real_path = malloc(sizeof(char)*(strlen(argv)+1));
      if(indexCourant->real_path == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
      sprintf(indexCourant->real_path,"%s",argv);
      if(args & CHECKSUM){
        if(!S_ISLNK(indexCourant->entete.mode))myMd5(indexCourant->real_path,indexCourant->entete.checksum);
      }
      //index[i].entete.checksum = //ADD Checksum
      //myMd5(indexCourant);
    }

/**
Fonction ajouter les élement donnée en parametre a l'index. archive complete ou liste de fichier
*/
Fichier * add_index_from_liste(char ** liste_argv, int args, int size, char * rep, int MAX_FICHIER, Fichier * index,int indice){
  int i, nb_argv = size;
  int nb_fichier = indice;
  for(i = 0; i< nb_argv; i++) {

    struct stat st;
    if(args & SIMLINK) { // A voir
      if(lstat(liste_argv[i], &st) != 0) {
        warn("le fichier  %s ne sera pas ajouter à l'archive: lstat fail \n",liste_argv[i]);
        continue;
      } // ADD ERROR
    }
    else {
      //index[nb_fichier].simlink = true;
      if(stat(liste_argv[i], &st) != 0) {
        warn("le fichier  %s ne sera pas ajouter à l'archive: stat fail \n",liste_argv[i]);
        continue;
      } // ADD ERROR
    }
    if(S_ISDIR(st.st_mode)) {
      //printf("1 :%d\n",nb_fichier);
      nb_fichier += creer_index_from_dir(liste_argv[i],liste_argv[i],st,&index,nb_fichier,&MAX_FICHIER,args, rep,true);
      //printf("2: %d\n",nb_fichier);
    }
    else if(S_ISREG(st.st_mode)||S_ISLNK(st.st_mode)) {
      creer_index_from_file(liste_argv[i],NULL,st,&index[nb_fichier],args, rep,true);
      if((args & SKIP_OLD) && (args & ADD)){
        last_modif(index,nb_fichier);
      }
      nb_fichier++;
    }
    if(nb_fichier == MAX_FICHIER) {
      MAX_FICHIER += 100;
      index = realloc(index,sizeof(Fichier)*MAX_FICHIER);
      if(index == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    }
  }
  index = realloc(index,sizeof(Fichier)*(nb_fichier+1));
  if(index == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
  index[nb_fichier].path = "\0";
  return index;
}

/**
Fonction créant l'index a partir des parametre donner par l'utilisateur
*/
Fichier* creer_index_from_liste(char ** liste_argv, int args, int size, char * rep){
  int MAX_FICHIER = 100;
  Fichier* index = malloc(sizeof(Fichier)*MAX_FICHIER);
  if(index == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
  int indice = 0;
  char * tmp = "";
  if (args & REP){

    get_file_name(rep, &tmp);
    //printf("%s : %s %d \n",rep,tmp,(strlen(tmp)));
    index[indice].path = malloc(sizeof(char)*(strlen(tmp)+1));
    if(index[indice].path == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    index[indice].real_path = malloc(sizeof(char)*(strlen(tmp)+1));
    if(index[indice].real_path == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    sprintf(index[indice].path,"%s",tmp);
    sprintf(index[indice].real_path,"%s",tmp);
    //index[indice].path  = tmp;
    //index[indice].real_path  = tmp;
    index[indice].entete.mode  = 0040000 | S_IRWXU;
    index[indice].entete.file_length = 0;
    index[indice].entete.path_length = strlen(tmp);
    indice++;
    //free(tmp);
  }
  return add_index_from_liste(liste_argv,args,size,tmp,MAX_FICHIER,index,indice);
}
