//
// Created by Lorenzo Cascone on 04/05/23.
//

#include "headers/Collector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/input_parser.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "headers/queue.h"

#define SOCK_PATH "./farm.sck"


void deleteSocket() {
    unlink(SOCK_PATH);
}


void collectorExecutor(int sockfd) {


    node_t *head = NULL;
    fd_set set, rdset;
    int continueLoop = 1;



    //accept
    int clientfd;
    if ((clientfd = accept(sockfd, NULL, NULL)) == -1) {
        perror("Error accepting");
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&set); //clear set
    FD_SET(clientfd, &set); //add clientfd to set

    int count = 0;
    while (continueLoop) {
        if (count == 4)continueLoop = 0;

        rdset = set;
        if (select(clientfd + 1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Error select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(clientfd, &rdset)) {
            long sumSent = 0;
            int pathSize = 0;
            int check = 0;
            if ((check = read(clientfd, &sumSent, sizeof(long))) == -1) {
                perror("Error reading sumSent\n");
                exit(EXIT_FAILURE);
            }
            if (check == 0)continue;
            //printf("Sum sent: %ld\n", sumSent);
            if ((check = read(clientfd, &pathSize, sizeof(int))) == -1) {
                perror("Error reading size of path\n");
                exit(EXIT_FAILURE);
            }
            if (check == 0)continue;
            //printf("Path size: %d\n", pathSize);
            char *buffer = malloc(sizeof(char) * pathSize + 1);
            if (buffer == NULL) {
                perror("Error allocating memory\n");
                exit(EXIT_FAILURE);
            }
            if (read(clientfd, buffer, pathSize) == -1) {
                perror("Error reading path\n");
                exit(EXIT_FAILURE);
            }
            buffer[pathSize] = '\0';
            push(&head, buffer, sumSent);
            free(buffer);
        }
        count++;
    }
    //print queue
    printQueue(head);

    //free queue
    freeQueue(&head);


}