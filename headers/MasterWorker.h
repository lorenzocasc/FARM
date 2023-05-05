//
// Created by Lorenzo Cascone on 02/05/23.
//



#ifndef SOLPROJECT_MASTERWORKER_H
#define SOLPROJECT_MASTERWORKER_H
#include "input_parser.h"
#include "threadpool.h"
#include <signal.h>


void * executeMasterWorker(int argc, char* argv[]);
long value(char *string);
void signalHandler(sigset_t *set);
static void handleSIGUSR1(int signal);
static void handleHIQTU(int signal);
#endif //SOLPROJECT_MASTERWORKER_H
