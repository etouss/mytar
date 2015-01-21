#ifndef __lock__
#define __lock__
#include "struct.h"
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


int lockfile(int fd, int type,Fichier * f);
int forcelockfile(int fd,int type);
boolean isEmptyLc();
Fichier popLc();
int lockfileP(int fd, int type,char * p);
boolean isEmptyLcP();
char* popLcP();
#endif
