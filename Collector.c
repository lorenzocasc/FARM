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
#include "headers/queue.h"

#define SOCK_PATH "./farm.sck"


void deleteSocket() {
    unlink(SOCK_PATH);
}


void collectorExecutor() {

    // cancello il socket file se esiste
    deleteSocket();
    // se qualcosa va storto ....
    atexit(deleteSocket);
    node_t *head = NULL;
    int sockfd;
    int continueLoop = 1;
    struct sockaddr_un server_addr;
    fd_set set, rdset;

    //Create socket
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    //Set server address
    memset(&server_addr, 0, sizeof(server_addr)); //clear struct
    server_addr.sun_family = AF_UNIX; //set family
    strcpy(server_addr.sun_path, SOCK_PATH); //set path

    //bind socket
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    //listen
    if (listen(sockfd, 1) == -1) {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }

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
        if(count == 5)continueLoop = 0;

        rdset = set;
        if (select(clientfd + 1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Error select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(clientfd, &rdset)) {
            long sumSent = 0;
            int pathSize = 0;
            int check = 0;
            if ((check=read(clientfd, &sumSent, sizeof(long))) == -1) {
                perror("Error reading sumSent\n");
                exit(EXIT_FAILURE);
            }
            if(check == 0)continue;
            //printf("Sum sent: %ld\n", sumSent);
            if((check=read(clientfd, &pathSize, sizeof(int))) == -1){
                perror("Error reading size of path\n");
                exit(EXIT_FAILURE);
            }
            if(check == 0)continue;
            //printf("Path size: %d\n", pathSize);
            char *buffer = malloc(sizeof(char)*pathSize+1);
            if(buffer == NULL){
                perror("Error allocating memory\n");
                exit(EXIT_FAILURE);
            }
            if (read(clientfd, buffer, pathSize) == -1) {
                perror("Error reading path\n");
                exit(EXIT_FAILURE);
            }
            buffer[pathSize] = '\0';
            //printf("Path: %s\n", buffer);
            push(&head, buffer, sumSent);
            free(buffer);
        }
        count++;
    }

    //close socket
    if(close(sockfd) == -1){
        perror("Error closing socket\n");
        exit(EXIT_FAILURE);
    }

    //print queue
    printQueue(head);

    //free queue
    freeQueue(&head);

    //delete socket at exit
    deleteSocket();
    atexit(deleteSocket);


}