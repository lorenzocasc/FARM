//
// Created by Lorenzo Cascone on 02/05/23.
//



#ifndef SOLPROJECT_MASTERWORKER_H
#define SOLPROJECT_MASTERWORKER_H
#include "input_parser.h"
#include "threadpool.h"
#include <signal.h>


void * executeMasterWorker(int argc, char* argv[], int pipefd, int pipeKill);
long value(char *string);
void signalHandler(sigset_t *set);
void handleSIGUSR1(int signal);
void handleHIQTU(int signal);
#endif //SOLPROJECT_MASTERWORKER_H
