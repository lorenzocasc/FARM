//
// Created by Lorenzo Cascone on 01/05/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/input_parser.h"
#include "../headers/MasterWorker.h"
#include "../headers/Collector.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCK_PATH "./farm.sck"

//delete socket file if exists
void deleteSocket() {
    unlink(SOCK_PATH);
}

int main(int argc, char *argv[]) {


    //pipe for communicating "quit" messages between master and collector
    //"quit" messages are sent by master to collector: 1) when it has finished its work and has to be terminated
    //                                                 2) when it has to be terminated because of a SIGINT, SIGQUIT,
    //                                                    SIGTERM and SIGHUP signal
    int pipefd[2];
    if(pipe(pipefd) == -1){
        perror("Error pipe");
        exit(EXIT_FAILURE);
    }


    //pipe for communicating "print" messages between master and collector
    //"print" messages are sent by master to collector when a SIGUSR1 signal is received
    int pipeKill[2];
    if(pipe(pipeKill) == -1){
        perror("Error pipe");
        exit(EXIT_FAILURE);
    }

    //Check if the number of arguments is correct
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <-n value> <-q value> <-d value> <-t value> <file-list>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // cancello il socket file se esiste
    deleteSocket();
    // se qualcosa va storto ....
    atexit(deleteSocket);

    int sockfd;
    struct sockaddr_un server_addr;

    //Create server socket for the Collector
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

    //create child process
    pid_t pid = fork();
    if (pid == -1) { //error
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) { //child
        close(pipefd[1]); //close write
        collectorExecutor(sockfd, pipefd[0],pipeKill[0]); //execute collector code
        close(sockfd); //close socket
        close(pipefd[0]); //close read
        close(pipeKill[0]); //close read
        exit(EXIT_SUCCESS);
    }
    if (pid > 0) { //parent
        close(pipefd[0]); //close read
        close(pipeKill[0]); //close read
        executeMasterWorker(argc, argv, pipefd[1],pipeKill[1]); //execute master-worker code
        close(pipefd[1]); //close write
        close (pipeKill[1]); //close write
    }
    exit(EXIT_SUCCESS);
}