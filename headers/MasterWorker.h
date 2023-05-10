//
// Created by Lorenzo Cascone on 02/05/23.
//



#ifndef SOLPROJECT_MASTERWORKER_H
#define SOLPROJECT_MASTERWORKER_H
#include "input_parser.h"
#include "threadpool.h"
#include <signal.h>

void * executeMasterWorker(int argc, char* argv[], int pipefd, int pipeKill); //Function to execute the MasterWorker
long value(char *string);  //Function to calculate the value of a file
void signalHandler(sigset_t *set);
void handleSIGUSR1(); //Function to handle SIGUSR1
void handleHIQTU(); //Function to handle SIGQUIT, SIGINT, SIGTERM, SIGHUP
int closeSocket(int socket); //Function to close the socket
#endif //SOLPROJECT_MASTERWORKER_H
