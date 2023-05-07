//
// Created by Lorenzo Cascone on 02/05/23.
//
#include <sys/socket.h>
#include "../headers/MasterWorker.h"
#include "../headers/input_parser.h"
#include "../headers/threadpool.h"
#include "../headers/utils.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/un.h>
#include <signal.h>

#define SOCK_PATH "./farm.sck"

int pipe_fd;
static long nThreads = 4;
static long queueSize = 8;
static long delay = 0;
threadpool_t *threadpool;

long value(char *string) {
    long sum = 0;
    long number;
    int i = 0;

    FILE *fp = fopen(string, "rb");
    if (fp == NULL) {
        perror("fopen");
        free(string);
        exit(EXIT_FAILURE);
    }

    while (fread(&number, sizeof(number), 1, fp) == 1) {
        sum += i * number;
        i++;
    }

    if (fclose(fp) != 0) {
        perror("Errore chiusura file binario");
        free(string);
        exit(EXIT_FAILURE);
    }

    free(string);
    return sum;
}

static void handleSIGUSR1(int signal) {
    printf("Received signal %d\n", signal);
    exit(EXIT_SUCCESS);
}
static void handleHIQTU(int signal) {
    printf("Received signal %d\n", signal);
    printf("Destroying threadpool in handle\n");
    destroyThreadPool(threadpool,0);
    printf("Threadpool destroyed in handle\n");
    char message = 'e';
    write(pipe_fd,&message,sizeof(char));
    exit(EXIT_SUCCESS);
}

void signalHandler(sigset_t *set) {

    //Set signal handler
    struct sigaction sa; //signal action
    sigfillset(set); //set all signals

    if(pthread_sigmask(SIG_SETMASK, set, NULL) != 0){
        perror("Error pthread_sigmask");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa)); //set sa to 0
    sa.sa_handler = handleSIGUSR1;
    if(sigaction(SIGUSR1, &sa, NULL) == -1){ //SIGUSR1
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = handleHIQTU;
    // handling SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1
    if (sigaction(SIGHUP, &sa, NULL) == -1) { //SIGHUP
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &sa, NULL) == -1) { //SIGINT
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGQUIT, &sa, NULL) == -1) { //SIGQUIT
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) { //SIGTERM
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }

    // handling SIGPIPE
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }

    sigemptyset(set);
    if(pthread_sigmask(SIG_SETMASK, set, NULL) != 0){
        perror("Error pthread_sigmask");
        exit(EXIT_FAILURE);
    }


}


void *executeMasterWorker(int argc, char *argv[], int pipefd) {
    struct sockaddr_un server_addr;
    int socket_fd;
    pipe_fd = pipefd;

    sigset_t set;
    signalHandler(&set);


    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("error creating socket in masterworker");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);

    int fd_c;
    char *path = NULL;
    int queue = 0; //flag that assumes 1 only if a queue of files is passed

    getConfigArgs(argc, argv, &nThreads, &queueSize, &path, &delay, &queue);

    if (path == NULL && queue == 0) {
        printf("Error: no directory or queue of files passed\n");
        exit(EXIT_FAILURE);

    }


    printf("--------------------\n");
    printf("nThreads: %ld\n", nThreads);
    printf("queueSize: %ld\n", queueSize);
    if (path != NULL)printf("path: %s\n", path);
    printf("delay: %ld\n", delay);
    printf("queue: %d\n", queue);
    printf("--------------------\n");


    //connect to socket of the collector, if it fails exit
    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("error connecting to socket in masterworker");
        exit(EXIT_FAILURE);
    }


    //create threadpool
    threadpool = createThreadPool(nThreads, queueSize, delay, socket_fd);

    //get files from the directory or the queue of files
    getArgs(argc, argv, threadpool, &queue, path);

    //Wait for threads to finish
    destroyThreadPool(threadpool, 0);

    char message = 'e'; //All args processed ending message
    write(pipe_fd,&message,sizeof(char)); //send the message to the collector

    close(socket_fd); //close socket
    //free memory
    free(path);
    return NULL;
}


