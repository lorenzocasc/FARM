//
// Created by Lorenzo Cascone on 04/05/23.
//



#ifndef SOLPROJECT_COLLECTOR_H
#define SOLPROJECT_COLLECTOR_H
#include <signal.h>
int collectorExecutor(int sockfd, int pipefd, int pipeKill);
void deleteSocket();
void sigHandler(sigset_t *set);
#endif //SOLPROJECT_COLLECTOR_H

