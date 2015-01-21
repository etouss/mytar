//
//  creation.h
//  myTar
//
//  Created by Etienne Toussaint on 30/11/2014.
//  Copyright (c) 2014 Etienne Toussaint. All rights reserved.
//

#ifndef __myTar__creation__
#define __myTar__creation__

#include <stdio.h>
#include "struct.h"
#include "md5.h"

Fichier* creer_index_from_liste(char ** liste_fichiers, int args, int size, char *);
Fichier * add_index_from_liste(char ** liste_argv, int args, int size, char * rep, int MAX_FICHIER, Fichier * index,int indice);


#endif /* defined(__myTar__creation__) */
