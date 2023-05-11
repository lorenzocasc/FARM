//
// Created by Lorenzo Cascone on 04/05/23.
//

#include "../headers/Collector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../headers/queue.h"
#include <sys/time.h>
#include <sys/types.h>
#include <pthread.h>
#include "../headers/utils.h"

#define SOCK_PATH "./farm.sck"

int continueLoop = 1; //flag
int flagSocket = 1; //flag
node_t *head = NULL; //create head of queue that will contain the results


void sigHandler(sigset_t *set) {

    struct sigaction sa; //create sigaction struct
    sigfillset(set); //fill set with all signals
    if (pthread_sigmask(SIG_SETMASK, set, NULL) != 0) { //set mask
        perror("Error pthread_sigmask");
        exit(EXIT_FAILURE);
    }
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN; //set handler to ignore
    if (sigaction(SIGPIPE, &sa, NULL) == -1) { //set handler to ignore SIGPIPE
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }
    sigemptyset(set); //empty set
    if (pthread_sigmask(SIG_SETMASK, set, NULL) != 0) {  //set mask
        perror("Error pthread_sigmask");
        exit(EXIT_FAILURE);
    }

    //add signals to set
    sigaddset(set, SIGUSR1);
    sigaddset(set, SIGHUP);
    sigaddset(set, SIGINT);
    sigaddset(set, SIGQUIT);
    sigaddset(set, SIGTERM);

    //the above signals will be blocked
    if (pthread_sigmask(SIG_BLOCK, set, NULL) != 0) { //set mask
        perror("Error pthread_sigmask");
        exit(EXIT_FAILURE);
    }

}

//function that will be executed by the collector process
int collectorExecutor(int sockfd, int pipefd) {

    fd_set set, rdset;
    int maxfd;
    sigset_t signalSet; //create signal set
    sigHandler(&signalSet); //set signal handler


    //accept connection from Master
    int clientfd;
    if ((clientfd = accept(sockfd, NULL, NULL)) == -1) {
        perror("Error accepting");
        exit(EXIT_FAILURE);
    }

    //set maxfd
    FD_ZERO(&set); //clear set
    FD_SET(clientfd, &set); //add clientfd to set
    FD_SET(pipefd, &set); //add pipefd to set
    maxfd = clientfd > pipefd ? clientfd : pipefd; //set maxfd


    while (continueLoop || flagSocket) { //loop until both pipe and socket are closed

        rdset = set;

        int d;
        //select on pipefd and clientfd
        if ((d = select(maxfd + 1, &rdset, NULL, NULL, NULL)) == -1) {
            perror("Error select");
            exit(EXIT_FAILURE);
        }


        for (int c = 0; c < maxfd + 1; c++) { //loop on all fd

            if (FD_ISSET(c, &rdset)) { //check if c is in rdset

                if (c == pipefd) {  //read from pipe
                    int len;
                    long check;

                    if ((check = read(pipefd, &len, sizeof(int))) ==
                        -1) {  //Reading len value, if 0 then pipe closed connection
                        perror("Error reading from pipe");
                        exit(EXIT_FAILURE);
                    }
                    if (check != 0) {
                        char *buffer = malloc(sizeof(char) * len + 1); //allocate memory for buffer
                        if (buffer == NULL) {
                            perror("Error allocating memory\n");
                            exit(EXIT_FAILURE);
                        }
                        if (read(pipefd, buffer, len) == -1) { //read from pipe
                            perror("Error reading from pipe");
                            free(buffer);
                            exit(EXIT_FAILURE);
                        }
                        buffer[len] = '\0';
                        if (strcmp(buffer, "quit") == 0) { //quit command
                            continueLoop = 0;
                            free(buffer);
                            continue;
                        }
                        if (strcmp(buffer, "print") == 0){
                            printQueue(head);
                            free(buffer);
                            continue;
                        }
                        free(buffer);
                    }
                }


                if (c == clientfd) {  //read from socket
                    long sumSent = 0;
                    int pathSize = 0;
                    int check, check1, check2;
                    if ((check = read(clientfd, &sumSent, sizeof(long))) ==
                        -1) {  //Reading sum value, if 0 then client closed connection
                        perror("Error reading sumSent\n");
                        exit(EXIT_FAILURE);
                    }
                    if (check == 0) {
                        flagSocket = 0;
                        continue;
                    }

                    if ((check1 = read(clientfd, &pathSize, sizeof(int))) ==
                        -1) { //Reading size of path string to allocate memory
                        perror("Error reading size of path\n");
                        exit(EXIT_FAILURE);
                    }
                    if (check1 == 0) {
                        flagSocket = 0;
                        continue;
                    }

                    char *buffer = malloc(sizeof(char) * pathSize + 1); //Allocating memory for path string
                    if (buffer == NULL) {
                        perror("Error allocating memory\n");
                        exit(EXIT_FAILURE);
                    }
                    if ((check2 = read(clientfd, buffer, pathSize)) == -1) {  //Reading path string
                        perror("Error reading path\n");
                        exit(EXIT_FAILURE);
                    }
                    if (check2 == 0) {
                        flagSocket = 0;
                        continue;
                    }
                    buffer[pathSize] = '\0';
                    pushOrdered(&head, buffer, sumSent); //pushing to <sum, path> into queue in order
                    continue;

                }
            }
        }
    }

    //print queue
    printQueue(head);
    //free queue
    freeQueue(&head);

    return 1;
}