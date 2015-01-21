//
//  md5.h
//  myTar
//
//  Created by Etienne Toussaint on 25/11/2014.
//  Copyright (c) 2014 Etienne Toussaint. All rights reserved.
//

#ifndef __myTar__md5__
#define __myTar__md5__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "struct.h"
#include <sys/types.h>
#include <sys/wait.h>


void myMd5(char * path,char * md5);
boolean myGzip(char * tar);
int myGunzipPipe(char * zip);
int myGunzipFile(char * path);
int myGunzipFile2(char * path);
void diff(char ** file,int size,Fichier index, int archive);

#endif /* defined(__myTar__md5__) */
