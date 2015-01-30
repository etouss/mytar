#include "archiver.h"


/**
Simple fonction rendant le maximum de deux int.
*/
int max(int a,int b){
        if(a>b) return a;
        else return b;
}

/**
Fonction écrivant dans une archive les fichiers contenue dans un index.
Si c'est dernier sont stocker sur le systeme.
*/
boolean add_archive(Fichier * fichiers, int args,int archive,char* mtr){
        char buf[BUFFSIZE];
        long lu;
        int input,i=0;
        boolean pile = false;
        while(42) {
                Fichier fic;
                if(pile || fichiers[i].path[0]==0){
                  pile = true;
                  if(!isEmptyLc())fic = popLc();
                  else break;
                }
                else fic = fichiers[i];
                if(fic.skip) {
                        i++;
                        continue;
                } // null
                if(fic.entete.file_length == 0) {
                        if(write(archive,&(fic.entete),sizeof(struct Entete))!=sizeof(struct Entete)) errxFree(fichiers,NULL,NULL,"Probleme d'écriture dans l'archive %s, archivage interompu \n", mtr);
                        if(write(archive,fic.path,fic.entete.path_length+1)!=fic.entete.path_length+1)errxFree(fichiers,NULL,NULL,"Probleme d'écriture dans l'archive %s, archivage interompu \n", mtr);
                        i++;
                        continue;
                }
                else if(args & SIMLINK && S_ISLNK(fic.entete.mode)) {
                        char buf[4096];
                        if(readlink(fic.real_path, buf, 4096) == -1) {
                                //if(errno != 0)perror(strerror(errno));
                                warn("impossible d archiver %s , probleme de readlink\n",fic.path);
                                i++;
                                //errno = 0;
                                continue;
                        }
                        if(write(archive,&(fic.entete),sizeof(struct Entete))!=sizeof(struct Entete)) errxFree(fichiers,NULL,NULL,"Probleme d'écriture dans l'archive %s, archivage interompu \n", mtr);
                        if(write(archive,fic.path,fic.entete.path_length+1)!=fic.entete.path_length+1) errxFree(fichiers,NULL,NULL,"Probleme d'écriture dans l'archive %s, archivage interompu \n", mtr);
                        if(write(archive,buf,fic.entete.file_length)!=fic.entete.file_length) errxFree(fichiers,NULL,NULL,"Probleme d'écriture dans l'archive %s, archivage interompu \n", mtr);
                        i++;
                        continue;
                }
                else if((input = open(fic.real_path,O_RDONLY))<0) {
                        warn("impossible d archiver %s ,probleme de open. \n",fic.path);
                        i++;
                        continue;
                }
                //breakpoint2();
                if(!pile && (lockfile(input, F_RDLCK,&fic) == -1)){
                    fprintf(stderr,"Liberer le verrou sur %s\n",fic.path);
                    //warn("impossible d archiver %s ,probleme de verrou \n",fic.path);
                    i++;
                    continue;
                }
                else if(pile && (forcelockfile(input,F_RDLCK) == -1)){
                  warn("impossible d archiver %s , vous n'avez pas liberer l'accée \n",fic.path);
                  i++;
                  continue;
                }
                if(write(archive,&(fic.entete),sizeof(struct Entete))!=sizeof(struct Entete)) errxFree(fichiers,NULL,NULL,"Probleme d'écriture dans l'archive %s, archivage interompu \n", mtr);
                if(write(archive,fic.path,fic.entete.path_length+1)!=fic.entete.path_length+1) errxFree(fichiers,NULL,NULL,"Probleme d'écriture dans l'archive %s, archivage interompu \n", mtr);
                while(42) {
                        lu = read(input,buf,BUFFSIZE);
                        if (write(archive,buf,lu) != lu) errxFree(fichiers,NULL,NULL,"Probleme d'écriture dans l'archive %s, archivage interompu \n", mtr);
                        if (lu < BUFFSIZE) break;
                }
                close(input);
                i++;
        }
        close(archive);
        if(args & GZIP) return myGzip(mtr);
        return false;

}

/**
Fonction creer l'archive.
*/
boolean creer_archive(Fichier* fichiers, int args, char* mtr){
        int archive;
        if(strcmp(mtr,"") == 0){
                archive = 1;
        if(forcelockfile(archive, F_WRLCK) == -1) errx(1,"impossible d ouvrir l archive %s pour cause de verrou\n",mtr);
        return add_archive(fichiers,args,archive,mtr);
        }
        else if((args & GZIP) && (args & CREAT)){
          int i = 0;
          char * mtr_gz = malloc(sizeof(char)*strlen(mtr)-2);
          for(i=0;i<strlen(mtr)-3;i++){
            mtr_gz[i]=mtr[i];
          }
          mtr_gz[i]=0;
          if((archive = open(mtr_gz,O_WRONLY | O_CREAT | O_TRUNC,0600)) < 0)
            errx(1,"impossible d ouvrir l archive %s \n",mtr);
          if(forcelockfile(archive, F_WRLCK) == -1) errx(1,"impossible d ouvrir l archive %s pour cause de verrou\n",mtr);
          boolean boole = add_archive(fichiers,args,archive,mtr_gz);
          free(mtr_gz);
          return boole;
        }
        else {
          if((archive = open(mtr,O_WRONLY | O_CREAT | O_TRUNC,0600)) < 0)
                  errx(1,"impossible d ouvrir l archive %s \n",mtr);
          if(forcelockfile(archive, F_WRLCK) == -1) errx(1,"impossible d ouvrir l archive %s pour cause de verrou\n",mtr);
          return add_archive(fichiers,args,archive,mtr);
        }

}

/**
Fonction écrivant dans une archive les fichiers contenue dans un index.
Si c'est dernier sont stocker dans une archive.
*/
boolean creer_archive_from_archive(Fichier * fichiers, int args,char * mtr){
        int archive;
        int newArchive;
        Entete entete;
        int indice = 0;
        char name[4096];
        int j = 0;
        int lu = 0;
        char buf[BUFFSIZE];
        //printf("1\n");
        char *tmp = NULL;
        //printf("1:%s\n",path);
        get_file_name(mtr, &tmp);
        char * filenl = malloc(sizeof(char)*(12+strlen(tmp)));
        if(filenl == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        sprintf(filenl,"/tmp/nl%s.tmp",tmp);
        //filenl[10+strlen(tmp)]=0;
        //printf("%s\n",filenl);
        if((newArchive = open(filenl,O_WRONLY | O_CREAT | O_TRUNC,0600)) <= 0)
                errxFree(fichiers,filenl,NULL,"impossible d ouvrir l'archive %s\n",mtr);
        if(forcelockfile(newArchive, F_WRLCK) == -1)  errxFree(fichiers,filenl,NULL,"impossible d ouvrir l archive %s pour cause de verrou\n",filenl);
        //free(filenl);

        if(args & GZIP) {
          char * file = malloc(sizeof(char)*(10+strlen(tmp)));
          if(file == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
          sprintf(file,"/tmp/%s.tmp",tmp);
                if((archive = open(file,O_RDONLY))==-1) errx(1,"impossible d ouvrir l archive %s \n",mtr);
          //printf("%s\n",file);
          free(file);
        }
        else{
                if((archive = open(mtr,O_RDONLY))==-1)
                  errx(1,"impossible d ouvrir l archive %s \n",mtr);
        }
        if(forcelockfile(archive, F_RDLCK) == -1) errxFree(fichiers,filenl,NULL,"impossible d ouvrir l archive %s pour cause de verrou\n",mtr);

        while(fichiers[indice].path[0]!=0) {
                //breakpoint();
                //printf("2\n");
                if(fichiers[indice].archive) {
                        //printf("3\n");
                        //printf("%lld\n",lseek(archive,0,SEEK_CUR));
                        if(read(archive,&entete,sizeof(Entete)) != sizeof(Entete)) errx(1, "Probleme de lecture sur l'archive %s modification interompu\n", mtr);
                        if(read(archive,name,entete.path_length+1)!= entete.path_length+1) errx(1, "Probleme de lecture sur l'archive %s modification interompu\n", mtr);
                        name[entete.path_length]=0;
                        //Care mm début de nom ?
                        if( (args&DELETE) && strncmp(name,fichiers[indice].path,max(fichiers[indice].entete.path_length,entete.path_length)) != 0) {
                                //breakpoint();
                                lseek(archive,entete.file_length,SEEK_CUR);
                                continue;
                        }
                        if(fichiers[indice].skip) {
                                lseek(archive,entete.file_length,SEEK_CUR);
                                indice++;
                                continue;
                        }
                        //printf("%s\n",name);
                        if(write(newArchive,&entete,sizeof(Entete)) != sizeof(Entete)) errx(1, "Probleme d'écriture dans l'archive %s, archivage interompu \n", filenl);
                        if(write(newArchive,name,entete.path_length+1)!= entete.path_length+1) errx(1, "Probleme d'écriture dans l'archive %s, archivage interompu \n", filenl);
                        for(j=0; j<entete.file_length/BUFFSIZE; j++) {
                                if((lu = read(archive,buf,BUFFSIZE))!=BUFFSIZE) errx(1, "Probleme de lecture sur l'archive %s modification interompu\n", mtr);
                                if (write(newArchive,buf,BUFFSIZE)!=lu) errx(1, "Probleme d'écriture dans l'archive %s, archivage interompu \n", filenl);
                        }
                        if(read(archive,buf,entete.file_length%BUFFSIZE) != entete.file_length%BUFFSIZE) errx(1, "Probleme de lecture sur l'archive %s modification interompu\n", mtr);
                        if(write(newArchive,buf,entete.file_length%BUFFSIZE) != entete.file_length%BUFFSIZE) errx(1, "Probleme d'écriture dans l'archive %s, archivage interompu \n", filenl);
                        indice++;
                }
                else if(args & ADD) {
                        //printf("4\n");
                        close(archive);
                        add_archive(&fichiers[indice],args,newArchive,filenl);
                        if(unlink(mtr) != 0) warn("Probleme de unlink avec le fichier %s",mtr);
                        if(args&GZIP) {
                                char * file = malloc(sizeof(char)*(4+strlen(filenl)));
                                if(file == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
                                sprintf(file,"%s.gz",filenl);
                                if(link(file, mtr)!=0) warn("Probleme de link entre les fichiers %s et %s \n",file,mtr);
                                free(file);
                        }
                        else{
                                if(link(filenl, mtr)!=0) warn("Probleme de link entre les fichiers %s et %s \n",filenl,mtr);
                        }
                        free(filenl);
                        return true;
                }
        }
        close(archive);
        close(newArchive);
        if(unlink(mtr) != 0) warn("Probleme de unlink avec le fichier %s",mtr);
        if(args & GZIP) {
          myGzip(filenl);
          char * file = malloc(sizeof(char)*(4+strlen(filenl)));
          if(file == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
          sprintf(file,"%s.gz",filenl);
          if(link(file, mtr)!=0) warn("Probleme de link entre les fichiers %s et %s \n",file,mtr);
          free(file);
        }
        else{
          if(link(filenl, mtr)!=0) warn("Probleme de link entre les fichiers %s et %s \n",filenl,mtr);
        }
        free(filenl);
        return true;
}
