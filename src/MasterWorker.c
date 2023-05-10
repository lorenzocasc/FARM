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

void handleSIGUSR1(int signal) {
    //destroyThreadPool(threadpool,1);
    char message[] = "print";
    int len = strlen(message);
    if(write(pipe_fd, &len, sizeof(int)) == -1){
        perror("error writing in the pipe\n");
        exit(EXIT_FAILURE);
    }
    if (write(pipe_fd, message, len) == -1) {
        perror("error writing in the socket");
        exit(EXIT_FAILURE);
    }
    //exit(EXIT_SUCCESS);
}
void handleHIQTU(int signal) {
    destroyThreadPool(threadpool,0);
    char message[] = "quit";
    int len = strlen(message);
    if(write(pipe_fd, &len, sizeof(int)) == -1){
        perror("error writing in the pipe\n");
        exit(EXIT_FAILURE);
    }
    if (write(pipe_fd, message, len) == -1) {
        perror("error writing in the socket");
        exit(EXIT_FAILURE);
    }
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

    char *path = NULL;
    int queue = 0; //flag that assumes 1 only if a queue of files is passed

    getConfigArgs(argc, argv, &nThreads, &queueSize, &path, &delay, &queue);

    if (path == NULL && queue == 0) {
        printf("Error: no directory or queue of files passed\n");
        exit(EXIT_FAILURE);

    }

    /*
    printf("--------------------\n");
    printf("nThreads: %ld\n", nThreads);
    printf("queueSize: %ld\n", queueSize);
    if (path != NULL)printf("path: %s\n", path);
    printf("delay: %ld\n", delay);
    printf("queue: %d\n", queue);
    printf("--------------------\n");
    */

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
    //printf("Threadpool destroyed\n");

    //send message "quit" to collector
    char message[] = "quit";
    int len = strlen(message);
    if(write(pipefd, &len, sizeof(int)) == -1){
        perror("error writing in the pipe\n");
        exit(EXIT_FAILURE);
    }
    if (write(pipefd, message, len) == -1) {
        perror("error writing in the socket");
        exit(EXIT_FAILURE);
    }


    //*****************
    close(socket_fd); //close socket, mi fa buggare la ricezione del messaggio nel collector a volte
    //****************

    //free memory
    free(path);

    return NULL;
}

//Devo 1) creare una nuova pipe per gestire solo il print
    // 2) nel collector gestisco la read da questa nuova pipe con un thread
    // 3) nel collector, devo impostare
    //    un flag nuovo, messo a uno quando le check della read dal socket
    // vanno a 1, cosi prima di uscire dal ciclo totale devo controllare
    // se hanno finito di leggere dalla socket e se continueloop è 0, fine.
