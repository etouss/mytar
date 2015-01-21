#ifndef __struct__
#define __struct__
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdarg.h>

/**
* Ces defines correspondes aux differentes options du programme.
* Pour facilité la gestion des argument avec des opérations bit à bit.
*/
#define QUIET           0x10000
#define CONCAT          0x8000
#define CAT             0x4000
#define USAGE           0x2000
#define DIFF            0x1000
#define FILE_ARCHIVE    0x800
#define CREAT           0x400
#define ADD             0x200
#define DELETE          0x100
#define LIST            0x80
#define EXTRACT         0x40
#define SKIP_EXISTE     0x20
#define SIMLINK         0x10
#define REP             0x8
#define CHECKSUM        0x4
#define SKIP_OLD        0x2
#define GZIP            0x1
//                      fcadlxksCvuz
#define DECOMPRESS  EXTRACT|SKIP_EXISTE
#define COMPRESS    CREAT|SIMLINK
#define XOR(a,b,c,d,e,f,g,h,j) ((a && !b && !c && !d && !e && !f && !g && !h && !j) || (!a && b && !c && !d && !e && !f && !g && !h && !j) || (!a && !b && c && !d && !e && !f && !g && !h && !j) || (!a && !b && !c && d && !e && !f && !g && !h && !j) || (!a && !b && !c && !d && e && !f && !g && !h && !j) || (!a && !b && !c && !d && !e && f && !g && !h && !j) || (!a && !b && !c && !d && !e && !f && g && !h && !j) || (!a && !b && !c && !d && !e && !f && !g && h && !j) || (!a && !b && !c && !d && !e && !f && !g && !h && j))
#define BUFFSIZE 1024


/**
Simple enum pour faciliter la lecture des boolean
*/
typedef enum boolean { false, true } boolean;

/**
Structure entete comme spécifier dans l'ennoncé.
*/
typedef struct Entete{
    size_t path_length;
    off_t file_length;
    mode_t mode;
    time_t m_time; // date modification
    time_t a_time; // date acces
    char checksum[32];
}Entete;



/**
Structure de l'index.
Contenant une entete, le path, le real_path (-C )
Et deux boolean pour préciser le type de fichier que c'est:
Cad : Est qu'il doit être désarchiver ou modifié ?
      Est qu'il appartient au systeme ou une archive.
*/
typedef struct Fichier{
    Entete entete;
    char *path;
    char *real_path;
    boolean archive;
    boolean skip;

}Fichier;


void free_index(Fichier* index);
//Fichier* creer_index_from_liste(char ** liste_fichiers, int args, int size, char *);
//void creer_index_from_file(char * argv,struct stat st,Fichier *indexCourant,int args,char * repC, boolean first);
//int creer_index_from_dir(char * argv, char * path, struct stat st,Fichier **index, int indice, int * MAX_FICHIER, int args, char * repC, boolean first);
//int lockfile(int fd,int type);
boolean list_archive(Fichier * index, char **, int size);
void get_file_name(char* path, char** name);
boolean show(char ** file, int size, char * path);
boolean show2(char ** file, int size, char * path);
int endsWith_mtr_or_mtrgz(char *src);
void errxFree(Fichier *toFree1,void *toFree2,void *toFree3, const char* format, ...);

#endif
//typedef struct Fichier Fichier;
