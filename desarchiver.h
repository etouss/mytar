#ifndef __desarchiver__
#define __desarchiver__
#include "struct.h"
#include <err.h>
#include <sys/stat.h>
#include "md5.h"
#include "lock.h"

#define BUFFSIZE 1024


#endif

boolean extraire_archive(char * tar, int args, char* rep,char ** liste,int size_liste);
