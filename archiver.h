#ifndef __archiver__
#define __archiver__
#include "struct.h"
#include <string.h>
#include <errno.h>
#include <err.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "md5.h"
#include "lock.h"


boolean creer_archive(Fichier*,int, char*);
boolean creer_archive_from_archive(Fichier * fichiers, int args,char * mtr);
#endif
