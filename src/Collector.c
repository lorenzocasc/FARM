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
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define SOCK_PATH "./farm.sck"

int continueLoop = 1;

void deleteSocket() {
    unlink(SOCK_PATH);
}

void sigHandler(sigset_t *set) {

    struct sigaction sa;
    sigfillset(set);
    if (pthread_sigmask(SIG_SETMASK, set, NULL) != 0) {
        perror("Error pthread_sigmask");
        exit(EXIT_FAILURE);
    }
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }
    sigemptyset(set);
    if (pthread_sigmask(SIG_SETMASK, set, NULL) != 0) {
        perror("Error pthread_sigmask");
        exit(EXIT_FAILURE);
    }
    sigaddset(set, SIGUSR1);
    sigaddset(set, SIGHUP);
    sigaddset(set, SIGINT);
    sigaddset(set, SIGQUIT);
    sigaddset(set, SIGTERM);

    if (pthread_sigmask(SIG_BLOCK, set, NULL) != 0) {
        perror("Error pthread_sigmask");
        exit(EXIT_FAILURE);
    }

}


void collectorExecutor(int sockfd, int pipefd) {

    node_t *head = NULL;
    fd_set set, rdset;
    int maxfd;
    sigset_t signalSet;
    sigHandler(&signalSet);


    //accept
    int clientfd;
    if ((clientfd = accept(sockfd, NULL, NULL)) == -1) {
        perror("Error accepting");
        exit(EXIT_FAILURE);
    }

    //set maxfd
    FD_ZERO(&set); //clear set
    FD_SET(clientfd, &set); //add clientfd to set
    FD_SET(pipefd, &set); //add pipefd to set
    maxfd = clientfd > pipefd ? clientfd : pipefd;


    while (continueLoop) {


        rdset = set;


        int d;
        if ((d = select(maxfd + 1, &rdset, NULL, NULL, NULL)) == -1) {
            perror("Error select");
            exit(EXIT_FAILURE);
        }


        for (int c = 0; c < maxfd + 1; c++) {

            if (FD_ISSET(c, &rdset)) {
                if (c == pipefd) {
                    int len;
                    long check;

                    if ((check = read(pipefd, &len, sizeof(int))) == -1) {
                        perror("Error reading from pipe");
                        exit(EXIT_FAILURE);
                    }
                    if (check != 0) {
                        char *buffer = malloc(sizeof(char) * len + 1);
                        if (buffer == NULL) {
                            perror("Error allocating memory\n");
                            exit(EXIT_FAILURE);
                        }
                        if (read(pipefd, buffer, len) == -1) {
                            perror("Error reading from pipe");
                            free(buffer);
                            exit(EXIT_FAILURE);
                        }
                        buffer[len] = '\0';
                        if (strcmp(buffer, "quit") == 0) {
                            continueLoop = 0;
                            free(buffer);
                            continue;
                        }
                        if (strcmp(buffer, "print") == 0) {
                            //continueLoop = 0;
                            printQueue(head);
                            free(buffer);
                            //freeQueue(&head);
                            //return;
                        }
                    }
                }


                if (c == clientfd) {
                    long sumSent = 0;
                    int pathSize = 0;
                    int check, check1, check2;
                    if ((check = read(clientfd, &sumSent, sizeof(long))) == -1) {
                        perror("Error reading sumSent\n");
                        exit(EXIT_FAILURE);
                    }
                    if (check == 0) {
                        continue;
                    }

                    //printf("Sum sent: %ld\n", sumSent);
                    if ((check1 = read(clientfd, &pathSize, sizeof(int))) == -1) {
                        perror("Error reading size of path\n");
                        exit(EXIT_FAILURE);
                    }
                    if (check1 == 0) {
                        continue;
                    }

                    //printf("Path size: %d\n", pathSize);
                    char *buffer = malloc(sizeof(char) * pathSize + 1);
                    if (buffer == NULL) {
                        perror("Error allocating memory\n");
                        exit(EXIT_FAILURE);
                    }
                    if ((check2 = read(clientfd, buffer, pathSize)) == -1) {
                        perror("Error reading path\n");
                        exit(EXIT_FAILURE);
                    }
                    if (check2 == 0) {
                        continue;
                    }
                    buffer[pathSize] = '\0';
                    pushOrdered(&head, buffer, sumSent);
                    //c--;
                    continue;

                }


            }

        }
    }

    //print queue
    printQueue(head);
    //free queue
    freeQueue(&head);

    return;

}