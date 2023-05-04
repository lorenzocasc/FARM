//
// Created by Lorenzo Cascone on 01/05/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/input_parser.h"
#include "headers/MasterWorker.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "./farm.sck"

void deleteSocket(){
    unlink(SOCK_PATH);
}


int main(int argc, char *argv[]) {

    //Check if the number of arguments is correct
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <-n value> <-q value> <-d value> <-t value> <file-list>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd;
    struct sockaddr_un server_addr;

    //Create socket
    if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    //Set server address
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);

    //bind socket
    if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    //listen
    if(listen(sockfd, 1) == -1){
        perror("Error listening");
        exit(EXIT_FAILURE);
    }

    //delete socket at exit
    deleteSocket();
    atexit(deleteSocket);

    pid_t pid = fork();
    if(pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if(pid == 0){

    }
    if(pid > 0){
       executeMasterWorker(argc, argv);
    }







}