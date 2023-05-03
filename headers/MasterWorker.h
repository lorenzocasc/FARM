//
// Created by Lorenzo Cascone on 02/05/23.
//



#ifndef SOLPROJECT_MASTERWORKER_H
#define SOLPROJECT_MASTERWORKER_H
#include "input_parser.h"
#include "threadpool.h"


void * executeMasterWorker(int argc, char* argv[]);
void *ciao(char *string);
int value(char *string);

#endif //SOLPROJECT_MASTERWORKER_H
