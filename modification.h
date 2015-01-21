//
//  modification.h
//  myTar
//
//  Created by Etienne Toussaint on 30/11/2014.
//  Copyright (c) 2014 Etienne Toussaint. All rights reserved.
//

#ifndef __myTar__modification__
#define __myTar__modification__

#include <stdio.h>
#include "struct.h"
#include "creation.h"
#include "lock.h"

Fichier* creer_index_from_archive(char* archive_existante, int args, char ** liste, int size_liste,char * repC);

boolean concat_archive(char* mtr, char ** files, int size, int args);

#endif /* defined(__myTar__modification__) */
