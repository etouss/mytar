#include "desarchiver.h"

/**
Fonction permetant l'extraction d'une archive donnée en parametre.
*/
boolean extraire_archive(char * tar, int args, char* rep,char ** liste,int size_liste){
        Entete et;
        char buf[BUFFSIZE];
        char nom_output[PATH_MAX];
        int archive,output,j;
        ssize_t lu;
        if(args & REP) {
                mkdir(rep, 0700);
        }
        if(args & GZIP) {
                archive = myGunzipFile(tar);
        }
        else if(strcmp(tar,"0") == 0){
                archive = 0;
        }
        else if((archive = open(tar,O_RDONLY))==-1){
            errx(1, "impossible d ouvrir l archive %s désarchivage interompu. \n ", tar);
        }
        if(lockfile(archive, F_RDLCK,NULL) == -1) errx(1, "problème de verrou sur %s lecture non autorisé, désarchivage interompu\n", tar);
        while(1) {
                lu = read(archive,&et,sizeof(Entete));
                if(lu ==0) break;
                if(lu != sizeof(Entete)) errx(1, "Probleme de lecture sur %s, désarchivage interompu (vérifié le format de l'archive)\n", tar);
                if(read(archive,nom_output,et.path_length+1)!=et.path_length+1)
                  errx(1, "Probleme de lecture sur %s ,désarchivage interompu (vérifié le format de l'archive)\n", tar);

                nom_output[et.path_length]=0;
                char * tmp;
                if(args & REP) {
                        tmp = malloc(sizeof(char)*(strlen(nom_output)+2+strlen(rep)));
                        if(tmp == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
                        sprintf(tmp, "%s/%s",rep,nom_output);
                }
                else {
                        tmp = nom_output;
                }
                if(S_ISDIR(et.mode) && et.file_length == 0 && (show(liste,size_liste,nom_output) || show2(liste,size_liste,nom_output))) {
                  mkdir(tmp,et.mode);
                }
                else if(!show(liste,size_liste,nom_output)){
                        lseek(archive,et.file_length,SEEK_CUR);
                }
                else if(et.file_length>0) {
                        struct stat st;
                        int stat_result = 0;
                        if(S_ISLNK(et.mode)) {
                                //printf("symlink\n");
                                stat_result = lstat(tmp, &st);
                        }
                        else{
                                stat_result = stat(tmp, &st);
                        }
                        //printf("%d\n",stat_result);
                        boolean skip_old = false;
                        boolean skip_existe = false;
                        if (stat_result == 0 && st.st_mtime > et.m_time && (args & SKIP_OLD)) { // VERIFIER !!!!!!
                                skip_old = true;
                        }
                        if (stat_result == 0 && (args & SKIP_EXISTE)) {
                                skip_existe = true;
                        }
                        if(skip_existe || skip_old) {
                                lseek(archive,et.file_length,SEEK_CUR);
                                warn("Fichier %s déja existant donc non dédesarchiver \n",tmp);
                        }
                        else if(S_ISLNK(et.mode)) {
                                char * buf = malloc(sizeof(char)*et.file_length);
                                if(buf == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
                                if((lu = read(archive,buf,et.file_length))!=et.file_length) errx(1, "Probleme de lecture sur %s désarchivage interompu (vérifié le format de l'archive)\n", tar);
                                if((stat_result == 0) && (unlink(tmp) != 0)) warn("Probleme de unlink avec le fichier %s",tmp);
                                if(symlink(buf,tmp)!=0) warn("Probleme de symlink entre les fichiers %s et %s \n",buf,tmp);
                                //printf("symlink\n");
                                free(buf);
                        }
                        else{
                                if((output = open(tmp,O_WRONLY|O_CREAT|O_TRUNC,et.mode))==-1){
                                      warn("impossible de creer ou d'écraser le fichier %s, il ne sera pas désarchiver. \n",tmp);
                                      lseek(archive,et.file_length,SEEK_CUR);
                                      continue;
                                      //A tester
                                }
                                if(forcelockfile(output, F_WRLCK) == -1){
                                  warn("impossible d'écrire le fichier %s pour cause de verrou, il ne sera pas désarchiver. \n",tmp);
                                  lseek(archive,et.file_length,SEEK_CUR);
                                  continue;
                                }
                                else{
                                        for(j=0; j<et.file_length/BUFFSIZE; j++) {
                                                if((lu = read(archive,buf,BUFFSIZE))!=BUFFSIZE) errx(1, "Probleme de lecture sur %s, désarchivage interompu (vérifier le format de l'archive)\n", tar);
                                                if (write(output,buf,BUFFSIZE)!=lu) warn("Probleme d'ecriture sur %s, fichier de sorti probalement corrumpu",tmp);
                                        }
                                        if(read(archive,buf,et.file_length%BUFFSIZE)!=et.file_length%BUFFSIZE) errx(1, "Probleme de lecture sur %s, désarchivage interompu (vérifié le format de l'archive)\n", tar);
                                        if(write(output,buf,et.file_length%BUFFSIZE)!=et.file_length%BUFFSIZE) warn("Probleme d'ecriture sur %s, fichier de sorti probalement corrumpu",tmp);
                                        close(output);
                                        if(args & CHECKSUM) {
                                                char md5[32];
                                                myMd5(tmp,md5);
                                                //printf("%s\n",md5);
                                                if(et.checksum[0]!=0 && strncmp(et.checksum,md5,32)!=0) warn("CheckSum du fichier %s invalide. Fichier probablement érroné.",tmp);
                                        }
                                }
                        }

                }
                if(args & REP) free(tmp);
        }
        close(archive);
        return true;
}
