#include <string.h>
#include "archiver.h"
#include "desarchiver.h"
#include "modification.h"
#include "creation.h"

int args = 0x0;

void cleanup(int args, char *mtr) {
    //while(wait(NULL)!=1){}
    char *tmp = NULL;
    get_file_name(mtr, &tmp);
    //printf("hey : %d\n",args);
    if ((args & GZIP) && (args & (ADD | DELETE | EXTRACT | LIST))) {
        char *file = malloc(sizeof(char) * (9 + strlen(tmp)));
        if(file == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        sprintf(file, "/tmp/%s.tmp", tmp);
        if (unlink(file) != 0)warn("Probleme de unlink avec le fichier %s",file);
        free(file);
    }
    if ((args & GZIP) && (args & (ADD | DELETE))) {
        char *file = malloc(sizeof(char) * (14 + strlen(tmp)));
        if(file == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        sprintf(file, "/tmp/nl%s.tmp.gz", tmp);
        if (unlink(file) != 0) warn("Probleme de unlink avec le fichier %s",file);
        //printf("%s\n",file);
        free(file);
    }
    else if (args & (ADD | DELETE)) {
        char *file = malloc(sizeof(char) * (11 + strlen(tmp)));
        if(file == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        sprintf(file, "/tmp/nl%s.tmp", tmp);
        if (unlink(file) != 0) warn("Probleme de unlink avec le fichier %s",file);
        //printf("%s\n",file);
        free(file);
    }
}

void compatible_args(){
    int decompresse = args & (EXTRACT);
    int compresse = args & (CREAT);
    int ajout = args & (ADD);
    int suppress = args & (DELETE);
    int list = args & (LIST);
    int diff = args & (DIFF);
    int usage = args & (USAGE);
    int cat = args & (CAT);
    int concat = args & (CONCAT);
    if (!XOR(compresse, decompresse, ajout, suppress, list, diff, usage, cat, concat)) {
        errx(2,"error de mode  \n");
    }
    if (compresse){
        //C u z v s
        if(args & (SKIP_EXISTE)){
            errx(2,"-k : imcompatible avec mode compression \n");
        }
    }
    else if(decompresse){
        if(args & (SIMLINK)){
            errx(2,"-s : imcompatible avec mode decompression \n");
        }
    }
    else if (ajout){
        if(args & (SKIP_EXISTE | REP)){
            errx(2,"-kC : imcompatible avec mode ajout \n");
        }
    }
    else if (suppress){
        if(args & (SKIP_EXISTE  | CHECKSUM |SKIP_OLD | SIMLINK | REP)){
            errx(2,"-kvCus : imcompatible avec mode supression \n");
        }
    }
    else if (list){
        if(args & (SKIP_EXISTE | CHECKSUM | SKIP_OLD | SIMLINK | REP)){
            errx(2,"-kCvus : imcompatible avec mode liste \n");
        }
    }
    else if (usage){
        if(args & (SKIP_EXISTE | GZIP | CHECKSUM | SKIP_OLD | SIMLINK | REP)){
            errx(2,"-kzCuvs : imcompatible avec mode usage \n");
        }
    }

        //
    else if (diff){
        if(args & (SKIP_EXISTE | CHECKSUM | SKIP_OLD | SIMLINK | REP)){
            errx(2,"-kCuvs : imcompatible avec mode diff \n");
        }
    }
    else if (cat){
        if(args & (SKIP_EXISTE  | SKIP_OLD | SIMLINK | REP)){
            errx(2,"-kCuvs : imcompatible avec mode cat \n");
        }
    }
    else if (concat){
        if(args & (SKIP_EXISTE | SKIP_OLD | SIMLINK | REP)){
            errx(2,"-kzCuvs : imcompatible avec mode concat \n");
        }
    }
    else {
        errx(2,"aucun mode sélectionné");
    }
}


/**
* Fonction de gestion des arguments
* sortie :
* char ** mtr fichier archive
* char **rep repertoir de l'option -C
* char ***rep liste des fichiers passer en argument
*/
int read_args(char *argument[], int taille, char **mtr, char **rep, char ***files) {

    int size = 0;
    *files = malloc(sizeof(char *) * taille);
    if(*files == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    int i, j;
    int error = 0;
    int file_option = 0; // 0 non mis 1 mis 2 mtr donné
    int rep_option = 0;
    boolean gzip_option = false;
    for (i = 1; i < taille; i++) {
        if (argument[i][0] == '-' && argument[i][1] == '-') {
            if (strcmp("--diff", argument[i]) == 0){ if (args & DIFF) error = -1; else args |= DIFF;}
            else if (strncmp("--usage", argument[i], 5) == 0){ if (args & USAGE) error = -1; else args |= USAGE;}
            else if (strncmp("--cat", argument[i], 5) == 0){ if (args & CAT) error = -1; else args |= CAT;}
            else if (strncmp("--concat", argument[i], 5) == 0){ if (args & CONCAT) error = -1; else args |= CONCAT;}
        }
        else if (argument[i][0] != '-') {
            if(file_option == 1 || rep_option){
                int tmp = endsWith_mtr_or_mtrgz(argument[i]);
                if(file_option == 1 && tmp > 0) {
                    file_option = 2;
                    if (tmp == 1){
                        *mtr = argument[i];

                    }
                    else if (tmp == 2){
                        *mtr = argument[i];
                        gzip_option = true;
                    }
                    continue;
                }
                else if (rep_option == 1) {
                    rep_option = 2;
                    *rep = argument[i];
                    continue;
                }
            }
            (*files)[size] = argument[i];
            size++;
        }
        else {
            for (j = 1; j < strlen(argument[i]); j++) {
                switch (argument[i][j]) {
                    case 'f':
                        if (args & FILE_ARCHIVE) error = -1; else args |= FILE_ARCHIVE;
                        file_option = 1;
                        break;
                    case 'c':
                        if (args & CREAT) error = -1; else args |= CREAT;
                        break;
                    case 'a':
                        if (args & ADD) error = -1; else args |= ADD;
                        break;
                    case 'd':
                        if (args & DELETE) error = -1; else args |= DELETE;
                        break;
                    case 'l':
                        if (args & LIST) error = -1; else args |= LIST;
                        break;
                    case 'x':
                        if (args & EXTRACT) error = -1; else args |= EXTRACT;
                        break;
                    case 'k':
                        if (args & SKIP_EXISTE) error = -1; else args |= SKIP_EXISTE;
                        break;
                    case 's':
                        if (args & SIMLINK) error = -1; else args |= SIMLINK;
                        break;
                    case 'C':
                        rep_option = true;
                        if (args & REP) error = -1; else args |= REP;
                        break;
                    case 'v':
                        if (args & CHECKSUM) error = -1; else args |= CHECKSUM;
                        break;
                    case 'u':
                        if (args & SKIP_OLD) error = -1; else args |= SKIP_OLD;
                        break;
                    case 'z':
                        if (args & GZIP) error = -1; else args |= GZIP;
                        break;
                    case 'h':
                        if (args & USAGE) error = -1; else args |= USAGE;
                        break;
                    case 'q':
                        if (args & QUIET) error = -1; else args |= QUIET;
                        break;
                    default:
                        error = -1;
                        break;
                }
            }
        }

    }
    compatible_args();
    if (file_option == 1 || ((args & (CREAT | EXTRACT)) == 0 && file_option == 1)){
        /*char t [PATH_MAX];
        write(1,"Chemin de l'archive : ",sizeof("Chemin de l'archive : "));
        int s = read(0,&t, PATH_MAX);
        t[s-1] = 0;
        s--;
        *mtr = malloc(sizeof(char) * (s));
        if(*mtr == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
        sprintf(*mtr, "%s",t);
        */

        //Memory leak , a parfaire.
        error = -1;
    }
    if ((args & CREAT) && !(args & FILE_ARCHIVE)){
        *mtr = "";

    }
    if((args & (EXTRACT)) && !(args & FILE_ARCHIVE)){
        *mtr = calloc(sizeof(char),(2));
        *mtr[0] = '0';
    }
    if (error == -1) {
        errx(2,"Erreur de mode \n");
    }
    if (!(gzip_option ^ !(args & (GZIP)))) {
        errx(2,"Erreur option gzip \n");
    }
    if(size > 0){
        *files = realloc(*files, sizeof(char *) * size);
        if(*files == NULL)errx(1,"Probleme d'allocation de mémoire, programme interompu\n");
    }
    //
    return size;
}

// free_index
int main(int argc, char *argv[]) {
    char *mtr;
    char *rep = "";
    char **files;
    int size = read_args(argv, argc, &mtr, &rep, &files);
    Fichier *index;
    mode_t mask = umask(0000);
    if (args & CREAT) {
        index = creer_index_from_liste(files, args, size, rep);
        creer_archive(index, args, mtr);
        free_index(index);
    }
    else if (args & DELETE) {
        index = creer_index_from_archive(mtr, args, files, size, rep);
        creer_archive_from_archive(index, args, mtr);
        free_index(index);
    }
    else if (args & ADD) {
        index = creer_index_from_archive(mtr, args, files, size, rep);
        creer_archive_from_archive(index, args, mtr);
        free_index(index);
    }
    else if (args & EXTRACT) {
        extraire_archive(mtr, args, rep, files, size);
    }
    else if (args & LIST) {
        index = creer_index_from_archive(mtr, args, files, size, rep);
        list_archive(index, files, size);
        free_index(index);
    }
    else if (args & USAGE) {
        int f = open("./README", O_RDONLY);
        char buff[BUFFSIZE];
        size_t lu;
        while ((lu = read(f, buff, BUFFSIZE))!=0){
            write(0, buff, lu);
        }
        free(files);
        close(f);
        umask(mask);
        return 0;
    }
    else if (args & DIFF) {
        index = creer_index_from_archive(mtr, args, files, size, rep);
        free_index(index);
    }
    else if (args & CAT) {
        index = creer_index_from_archive(mtr, args, files, size, rep);
        free_index(index);
    }
    else if (args & CONCAT) {
        concat_archive(mtr, files, size, args);

    }
    if(mtr != NULL && !(args & QUIET)){
        if((args & (CREAT | ADD | DELETE)))
            printf("L'archive %s a été créé\n", mtr);
        else if((args & (EXTRACT))) printf("L'archive %s a décompressé\n", mtr);
    }
    cleanup(args, mtr);
    free(files);
    umask(mask);
    return 0;
}
