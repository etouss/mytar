//
//  md5.c
//  myTar
//
//  Created by Etienne Toussaint on 25/11/2014.
//  Copyright (c) 2014 Etienne Toussaint. All rights reserved.
//

#include "md5.h"

/**
Simple fonction permetant le calcul du maximum désolé du doublon.
*/

int max2(int a,int b){
  if(a>b) return a;
  else return b;
}

//MD5 lien symbolique ???


/**
Fonction calculant l'empreinte md5 d'un fichier path et l'écrivant
dans l'entete le cas échéant
*/
void myMd5(char * path,char * md5){
    int pipeDes[2];
    char * argv[3];
    pid_t p;
    if (pipe(pipeDes) == -1){
        warn("Erreur avec pipe() md5 du fichier %s impossible\n",path);
        // Est qu'il fait ecrire du vide je crois pas.
        return;
    }
    p = fork();
    if(p == -1){
      warn("Erreur avec Fork() md5 du fichier %s impossible\n",path);
      return;
    }
    if(p == 0){
        close(pipeDes[0]);
        argv[0] = "md5sum";
        //argv[1] = "-q";
        argv[1] = path;
        argv[2] = NULL;
        if(dup2(pipeDes[1], 1) ==-1)errx(1,"Probleme avec dup2 pipe corrumpu ? \n");
        close(pipeDes[1]);
        if(execvp("md5sum",argv) ==-1){
          exit(255);
        }
    }
    //wait(NULL);
    int statut;
    close(pipeDes[1]);
    if(read(pipeDes[0], md5, 32)!=32)warn("Erreur de lecture sur le pipe(), md5 du fichier %s impossible\n",path);
    close(pipeDes[0]);
    if(wait(&statut) == -1)errx(1,"Probleme avec wait md5 bizarre\n");
    if(WIFEXITED(statut)){
      if((WEXITSTATUS(statut) == 255))  warn("Probleme avec execvp md5 sur %s \n",path);
    }
    else if(WIFSIGNALED(statut)) warn("Probleme avec md5 sur %s md5 stopped with signal : %d \n",path,WTERMSIG(statut));
}

/**
Fonction zippant une archive tar.
*/
boolean myGzip(char * tar){
  //printf("here :: %s\n",tar);
  char * argv[3];
  pid_t p;
  p = fork();
  if(p == -1){
    errx(1, "Erreur avec fork() gzip de %s non effectuer, archivage intérrompu\n",tar);
  }
  if(p == 0){
    argv[0] = "gzip";
    //argv[1] = "-N";
    argv[1] = tar;
    argv[2] = NULL;
    if(execvp("gzip",argv) ==-1){
      exit(255);
    }
  }
  int statut;
  if(wait(&statut) == -1)errx(1,"Probleme avec wait gzip bizarre\n");
  if(WIFEXITED(statut)){
    if((WEXITSTATUS(statut) == 255))  errx(1,"Probleme avec execvp gzip sur %s \n",tar);
  }
  else if(WIFSIGNALED(statut)) errx(1,"Probleme avec gzip sur %s gzip stopped with signal : %d \n",tar,WTERMSIG(statut));
  return true;
}

/*int myGunzipPipe(char * path){
  int pipeDes[2];
  char * argv[5];
  pid_t p;
  if (pipe(pipeDes) == -1){
    errx(1,"Erreur avec pipe()\n");
  }
  p = fork();
  if(p == -1){
    errx(1, "Erreur avec fork()\n");
  }
  if(p == 0){
    close(pipeDes[0]);
    argv[0] = "gunzip";
    argv[1] = "-k";
    argv[2] = "-c";
    argv[3] = path;
    argv[4] = NULL;
    dup2(pipeDes[1], 1);
    close(pipeDes[1]);
    execvp("gunzip",argv);
  }
  close(pipeDes[1]);
  return pipeDes[0];
}
*/

/**
Fonction dézipant une archive passer en parametre dans un fichier temporaire.
Nous avons choisi un fichier temporaire plutot qu'un pipe ou un tube en raison
de la size limité de ces dernier entrainant des soucis pour la lecture de block
de 1024 ou des lseek plus important.
*/
int myGunzipFile(char * path){
  int pipeDes[2];
  char * argv[5];
  pid_t p;
  char *tmp = NULL;
  //printf("1:%s\n",path);
  get_file_name(path, &tmp);
  char * file = malloc(sizeof(char)*(10+strlen(tmp)));
  if(file == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
  sprintf(file,"/tmp/%s.tmp",tmp);
  if((pipeDes[1] = open(file,O_WRONLY | O_CREAT | O_TRUNC,0600)) <= 0)
    errx(1, "impossible d ouvrir le fichier temporaire pour gunziper %s désarchivage interompu. \n ", file);
  /*if(lockfile(pipeDes[1], F_WRLCK) == -1){
    errx(1, "impossible d ouvrir le fichier temporaire pour gunziper %s, pour cause de verrou désarchivage interompu. \n ", file);
  }*/
  if((pipeDes[0] = open(file,O_RDONLY)) <= 0)
    errx(1, "impossible d ouvrir le fichier temporaire pour gunziper %s désarchivage interompu. \n ", file);
  /*if(lockfile(pipeDes[0], F_RDLCK) == -1){
    errx(1, "impossible d ouvrir le fichier temporaire pour gunziper %s, pour cause de verrou désarchivage interompu. \n ", file);
  }*/
  //printf("%s\n",file);
  free(file);
  p = fork();
  if(p == -1){
    errx(1, "Erreur avec fork() gunzip de %s non effectuer, désarchivage intérrompu\n",path);
  }
  if(p == 0){
    close(pipeDes[0]);
    argv[0] = "gunzip";
    argv[1] = "-k";
    argv[2] = "-c";
    argv[3] = path;
    argv[4] = NULL;
    dup2(pipeDes[1], 1);
    close(pipeDes[1]);
    if(execvp("gunzip",argv) ==-1){
      exit(255);
    }
  }
  int statut;
  close(pipeDes[1]);
  if(wait(&statut) == -1)errx(1,"Probleme avec wait gunzip bizarre\n");
  if(WIFEXITED(statut)){
    if((WEXITSTATUS(statut) == 255))  errx(1,"Probleme avec execvp gunzip sur %s \n",path);
  }
  else if(WIFSIGNALED(statut)) errx(1,"Probleme avec gunzip sur %s gunzip stopped with signal : %d \n",path,WTERMSIG(statut));
  return pipeDes[0];
}

/**
Fonction affichant sur la sortie standart la diffenrence entre un fichier systeme
et un fichier de l'archive passer en parametre.
*/
void diff(char ** file,int size,Fichier index, int archive){
  if (size == 0) return;
  int i;
  int pipeDes[2];
  char * argv[5];
  argv[0] = "diff";
  argv[1] = "-u";
  argv[2] = "-";
  argv[3] = "-";
  argv[4] = NULL;
  pid_t p;
  //printf("%s\n",index.path);
  for(i = 0; i<size; i++){
      if(strncmp(index.path, file[i],max2(strlen(file[i]),strlen(index.path))) == 0){
        if (pipe(pipeDes) == -1){
          warn("Erreur avec pipe() diff du fichier %s impossible\n",index.path);
          // Est qu'il fait ecrire du vide je crois pas.
          continue;
        }
        p = fork();
        if(p == -1){
          warn("Erreur avec fork() diff du fichier %s impossible\n",index.path);
          // Est qu'il fait ecrire du vide je crois pas.
          continue;
        }
        if(p == 0){
          close(pipeDes[1]);
          argv[2] = file[i];
          dup2(pipeDes[0], 0);
          close(pipeDes[0]);
          if(execvp("diff",argv) ==-1){
            exit(255);
          }
        }
        close(pipeDes[0]);
        int lu = 0;
        int j = 0;
        int statut;
        char buf[BUFFSIZE];
        //geston de ces erreurs ?
        for(j=0; j<index.entete.file_length/BUFFSIZE; j++) {
          if((lu = read(archive,buf,BUFFSIZE))!=BUFFSIZE) err(1,"Probleme de lecture");
          if (write(pipeDes[1],buf,BUFFSIZE)!=lu) err(1,"Probleme d'ecriture");
        }
        if(read(archive,buf,index.entete.file_length%BUFFSIZE)!=index.entete.file_length%BUFFSIZE) err(1,"Probleme de lecture");
        if(write(pipeDes[1],buf,index.entete.file_length%BUFFSIZE)!=index.entete.file_length%BUFFSIZE) err(1,"Probleme d'ecriture");
        close(pipeDes[1]);
        if(wait(&statut) == -1)errx(1,"Probleme avec wait diff bizarre\n");
        if(WIFEXITED(statut)){
          if((WEXITSTATUS(statut) == 255))  warn("Probleme avec execvp diff sur %s \n",file[i]);
        }
        else if(WIFSIGNALED(statut)) warn("Probleme avec diff sur %s diff stopped with signal : %d \n",file[i],WTERMSIG(statut));
        break;
    }
  }
}


int myGunzipFile2(char * path){
  int pipeDes[2];
  char * argv[5];
  pid_t p;
  char *tmp = NULL;
  //printf("1:%s\n",path);
  get_file_name(path, &tmp);
  char * file = malloc(sizeof(char)*(9+strlen(tmp)));
  if(file == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
  sprintf(file,"/tmp/%s.tmp",tmp);
  if((pipeDes[1] = open(file,O_WRONLY | O_CREAT | O_TRUNC,0600)) <= 0)
    errx(1, "impossible d ouvrir le fichier temporaire pour gunziper %s désarchivage interompu. \n ", file);
    /*if(lockfile(pipeDes[1], F_WRLCK) == -1){
    errx(1, "impossible d ouvrir le fichier temporaire pour gunziper %s, pour cause de verrou désarchivage interompu. \n ", file);
  }*/
  if((pipeDes[0] = open(file, O_WRONLY|O_APPEND)) <= 0)
    errx(1, "impossible d ouvrir le fichier temporaire pour gunziper %s désarchivage interompu. \n ", file);
    /*if(lockfile(pipeDes[0], F_RDLCK) == -1){
    errx(1, "impossible d ouvrir le fichier temporaire pour gunziper %s, pour cause de verrou désarchivage interompu. \n ", file);
  }*/
  //printf("%s\n",file);
  free(file);
  p = fork();
  if(p == -1){
    errx(1, "Erreur avec fork() gunzip de %s non effectuer, désarchivage intérrompu\n",path);
  }
  if(p == 0){
    close(pipeDes[0]);
    argv[0] = "gunzip";
    argv[1] = "-k";
    argv[2] = "-c";
    argv[3] = path;
    argv[4] = NULL;
    dup2(pipeDes[1], 1);
    close(pipeDes[1]);
    if(execvp("gunzip",argv) ==-1){
      exit(255);
    }
  }
  int statut;
  close(pipeDes[1]);
  if(wait(&statut) == -1)errx(1,"Probleme avec wait gunzip bizarre\n");
  if(WIFEXITED(statut)){
    if((WEXITSTATUS(statut) == 255))  errx(1,"Probleme avec execvp gunzip sur %s \n",path);
  }
  else if(WIFSIGNALED(statut)) errx(1,"Probleme avec gunzip sur %s gunzip stopped with signal : %d \n",path,WTERMSIG(statut));
  return pipeDes[0];
}

/*
int myGunzip(char * zip){
  int pipeDes[2];
  char * argv[5];
  pid_t p;
  p = fork();
  if (pipe(pipeDes) == -1){
    errx(1,"Erreur avec pipe()\n");
  }
  if(p == -1){
    errx(1, "Erreur avec fork()\n");
  }
  if(p == 0){
    close(pipeDes[0]);
    argv[0] = "gunzip";
    argv[1] = "-k";
    argv[2] = "-c";
    argv[3] = zip;
    argv[4] = NULL;
    dup2(pipeDes[1], 1);
    close(pipeDes[1]);
    execvp("gunzip",argv);
  }
  close(pipeDes[1]);
  wait(NULL);
  return pipeDes[0];
  //Close, car renvoie descripteur
}
*/

/*
argv[0] = "gunzip";
argv[1] = "-k";
argv[2] = "-c";
argv[3] = zip;
argv[4] = NULL;
*/

/*
struct flock fl;
fl.l_type = F_WRLCK;
fl.whence = SEEK_CUR;
fl.l_start=0;
fl.l_len=0;
demande non bloquante de poser un verrou
sur tout le fichier
if( fcntl(desc, F_SETLK, &fl) == -1 ){
  if( errno==EAGAIN){
     verrouillage impossible,
    deja un verrou pose par un autre processus
    essayez plus tard encore une fois
  }
  else{
     tentative echoue pour une autre raison
    * probablement terminer le programme te regarder quel probleme

  }
}
enlever le verrou
fl.l_type=F_UNLCK;
fcntl(desc, F_SETLK, &fl)
*/
