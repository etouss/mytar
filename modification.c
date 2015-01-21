//
//  modification.c
//  myTar
//
//  Created by Etienne Toussaint on 30/11/2014.
//  Copyright (c) 2014 Etienne Toussaint. All rights reserved.
//

#include "modification.h"

/**
Fonction creeer un l'index des fichier contenue dans une archive.
*/
Fichier *creer_index_from_archive(char *archive_existante, int args, char **liste, int size_liste, char *repC) {
    int MAX_FICHIER = 100;
    Fichier *index = malloc(sizeof(Fichier) * MAX_FICHIER);
    if(index == NULL)errxFree(index,NULL,NULL,"Probleme d'allocation de mémoire, programme interompu\n");
    int nb_fichier = 0;
    int archive;
    ssize_t lu;
    if (args & GZIP) {
        archive = myGunzipFile(archive_existante);
    }
    else {
        if ((archive = open(archive_existante, O_RDONLY)) == -1)
          errxFree(index,NULL,NULL, "impossible d ouvrie l archive %s modifiication impossible. \n ", archive_existante);
    }
    if (forcelockfile(archive, F_RDLCK) == -1) errxFree(index,NULL,NULL, "problème de verrou sur %s lecture non autorié modification interompu\n", archive_existante);
    //if(lockfile(archive, F_RDLCK) == -1)errx(1,"problème de verrou1 %s\n",archive_existante);
    while (42) {
        lu = read(archive, &index[nb_fichier].entete, sizeof(Entete));
        if (lu == 0) break;

        index[nb_fichier].path = malloc((index[nb_fichier].entete.path_length) * sizeof(char));
        if(index[nb_fichier].path == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");

        if ((lu = read(archive, index[nb_fichier].path, index[nb_fichier].entete.path_length+1)) != index[nb_fichier].entete.path_length+1) {
            errx(1, "Probleme de lecture sur %s modification interompu\n", archive_existante);
        }
        else if (args & CAT) {
            int i;
            boolean test = false;
            for (i = 0; i < size_liste; i++) {
                if (show(liste, size_liste, index[nb_fichier].path)) {
                    int j = 0;
                    char buf[BUFFSIZE];
                    for (j = 0; j < index[nb_fichier].entete.file_length / BUFFSIZE; j++) {
                        if ((lu = read(archive, buf, BUFFSIZE)) != BUFFSIZE) errx(1, "Probleme de lecture sur %s modification interompu\n", archive_existante);
                        if (write(1, buf, BUFFSIZE) != lu) warn("Probleme d'écriture sur la sortie standard pour %s \n",index[nb_fichier].path);
                    }
                    if (read(archive, buf, index[nb_fichier].entete.file_length % BUFFSIZE) != index[nb_fichier].entete.file_length % BUFFSIZE) errx(1, "Probleme de lecture sur %s modification interompu\n", archive_existante);
                    if (write(1, buf, index[nb_fichier].entete.file_length % BUFFSIZE) != index[nb_fichier].entete.file_length % BUFFSIZE) warn("Probleme d'écriture sur la sortie standard pour %s \n",index[nb_fichier].path);
                    test = true;
                    break;
                }
            }
            if (!test)lseek(archive, index[nb_fichier].entete.file_length, SEEK_CUR);
        }
        else if (args & DIFF) {
            int i;
            boolean test = false;
            for (i = 0; i < size_liste; i++) {
                if (show(liste, size_liste, index[nb_fichier].path)) {
                    diff(liste, size_liste, index[nb_fichier], archive);
                    test = true;
                    break;
                }
            }
            if (!test)lseek(archive, index[nb_fichier].entete.file_length, SEEK_CUR);
        }
        else lseek(archive, index[nb_fichier].entete.file_length, SEEK_CUR);
        index[nb_fichier].archive = true;
        if (args & DELETE) {
            int i;
            for (i = 0; i < size_liste; i++) {
                if (show(liste, size_liste, index[nb_fichier].path)) {
                    free(index[nb_fichier].path);
                    free(index[nb_fichier].real_path);
                    nb_fichier--;
                }
            }
        }
        nb_fichier++;

        if (nb_fichier == MAX_FICHIER) {
            MAX_FICHIER += 100;
            index = realloc(index, sizeof(Fichier) * MAX_FICHIER);
            if(index == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        }
    }
    //if(args&GZIP) wait(NULL);
    close(archive);
    if (args & ADD) {
        //printf("ADD");
        return add_index_from_liste(liste, args, size_liste, repC, MAX_FICHIER, index, nb_fichier);
    }
    index = realloc(index, sizeof(Fichier) * (nb_fichier + 1));
    if(index == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    index[nb_fichier].path = "\0";
    return index;
}

/**
Fonction permetant de contaner des archives.
*/
boolean concat_archive(char* mtr, char ** files, int size, int args){
  //Ajouter test sit mtr est un gzip
  int archive,i=0, tmp ,k, lu;
  char buff[BUFFSIZE];
  if (args & GZIP) {
    archive = myGunzipFile2(mtr);
  }
  else {
    if((archive = open(mtr,O_WRONLY | O_CREAT | O_APPEND,0600)) <= 0)
      errx(1, "impossible d ouvrie l archive %s concatenation impossible. \n ", mtr);
    }
    //printf("%d\n",archive);
    if(forcelockfile(archive, F_WRLCK) == -1) errx(1, "problème de verrou sur %s ecriture non autorisé concatenation interompu\n", mtr);
    boolean pile = false;

    while(42){
      char * path;
      if(pile || i>=size){
        pile = true;
        if(!isEmptyLcP())path = popLcP();
        else break;
      }
      else path = files[i];
      k = endsWith_mtr_or_mtrgz(path);
      if( k == 0){
        warn("impossible de concatener %s",path);
        continue;
      }
      else if (k == 2){
        tmp = myGunzipFile(path);
      }
      else{
        if ((tmp = open(path, O_RDONLY,0600)) <= 0) errx(1,"impossible d ouvrir l'archive %s\n",path);
      }

      if(!pile && (lockfileP(tmp,F_RDLCK,path)==-1)){printf("Liberer le verrou sur %s \n",path);close(tmp);i++;continue;}
        else if(pile && forcelockfile(tmp, F_RDLCK) == -1) errx(1,"problème de verrou3 %s\n",path);

        while(1){
          if ((lu=read(tmp, buff, BUFFSIZE)) == 0) break;
          write(archive, buff, lu);
        }
        close(tmp);
        if (k == 2){
          char *tmp = NULL;
          get_file_name(path, &tmp);
          char * file = malloc(sizeof(char)*(9+strlen(tmp)));
          if(file == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
          sprintf(file,"/tmp/%s.tmp",tmp);
          unlink(file);
          free(file);
        }
        i++;


      }
      close(archive);
      if (args & GZIP) {
        unlink(mtr);
        char *tmp = NULL;
        //printf("1:%s\n",path);
        get_file_name(mtr, &tmp);
        char * file = malloc(sizeof(char)*(9+strlen(tmp)));
        if(file == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        sprintf(file,"/tmp/%s.tmp",tmp);
        myGzip(file);

        char * file2 = malloc(sizeof(char)*(strlen(file)+4));
        sprintf(file2,"%s.gz",file);
        free(file);
        if(link(file2, mtr)==-1)printf("%s",strerror(errno));
        unlink(file2);
        free(file2);

      }
      return true;
    }
