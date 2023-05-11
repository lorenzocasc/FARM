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

int pipe_fd;  //pipe for the master-collector communication, used to send "quit" message
int socketfd; //socket for the master-collector communication
static long nThreads = 4; //default thread number
static long queueSize = 8; //default queue size
static long delay = 0; //default delay
threadpool_t *threadpool; //threadpool

volatile sig_atomic_t quit = 0; //variable used to check if the program has to be terminated

//function that calculates the result number of a file
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

//Function that writes the "print" message in the pipe at SIGUSR1 signal
void handleSIGUSR1() {
    char message[] = "print";
    int len = strlen(message);
    if(write(pipe_fd, &len, sizeof(int)) == -1){ //write the length of the message
        perror("error writing in the pipe\n");
        exit(EXIT_FAILURE);
    }
    if (write(pipe_fd, message, len) == -1) { //write the message
        perror("error writing in the socket");
        exit(EXIT_FAILURE);
    }
}

//Function that writes the "quit" message in the pipe at SIGHUP, SIGINT, SIGQUIT, SIGTERM signal
void handleHIQTU() {
    quit = 1; //set the quit variable to 1
    char message[] = "quit";
    int len = strlen(message);
    if(write(pipe_fd, &len, sizeof(int)) == -1){  //write the length of the message
        perror("error writing in the pipe\n");
        exit(EXIT_FAILURE);
    }
    if (write(pipe_fd, message, len) == -1) {  //write the message
        perror("error writing in the socket");
        exit(EXIT_FAILURE);
    }
}
//function to close the socket, returns 1 if successful, 0 otherwise
int closeSocket(int socket){
    if (close(socket) == -1) {
        perror("Error close socket");
        return 0;
    }
    return 1;
}

//Signal handler function, to register the signals to be caught
void signalHandler(sigset_t *set) {

    //Set signal handler
    struct sigaction sa; //signal action
    sigfillset(set); //creating a mask that blocks all signals

    if(pthread_sigmask(SIG_SETMASK, set, NULL) != 0){
        perror("Error pthread_sigmask");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa)); //set sa to 0
    sa.sa_handler = handleSIGUSR1; //handling SIGUSR1, used to print the result
    if(sigaction(SIGUSR1, &sa, NULL) == -1){ //SIGUSR1
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }

    sa.sa_handler = handleHIQTU; //handling SIGHUP, SIGINT, SIGQUIT, SIGTERM, used to quit the program

    // register signals to be caught by the handler SIGHUP, SIGINT, SIGQUIT and SIGTERM
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
    sa.sa_handler = SIG_IGN; //ignore SIGPIPE
    if (sigaction(SIGPIPE, &sa, NULL) == -1) { //SIGPIPE
        perror("Error sigaction");
        exit(EXIT_FAILURE);
    }

    sigemptyset(set); //empty the signal set
    if(pthread_sigmask(SIG_SETMASK, set, NULL) != 0){
        perror("Error pthread_sigmask");
        exit(EXIT_FAILURE);
    }

}

//Master-worker function, called by the main function
// - argc, argv, pipefd, pipeKill : arguments passed from the main function
// - returns NULL
// - the function creates a socket and connects to the collector, then it creates a threadpool
void *executeMasterWorker(int argc, char *argv[], int pipefd) {

    //////////// Config variables ////////////
    struct sockaddr_un server_addr; //server address
    int socket_fd; //socket file descriptor
    pipe_fd = pipefd; //global variable pipe_fd = pipefd
    char *path = NULL; //path of the directory passed as argument if present
    int queue = 0; //flag that assumes 1 only if a queue of files is passed

    sigset_t set; //signal set
    signalHandler(&set); //call the signal handler function

    //////////// Socket creation ////////////
    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("error creating socket in masterworker");
        exit(EXIT_FAILURE);
    }


    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);

    //calling a function that returns config values for the threadpool
    getConfigArgs(argc, argv, &nThreads, &queueSize, &path, &delay, &queue);

    //if no directory or queue of files is passed, exit
    if (path == NULL && queue == 0) {
        printf("Error: no directory or queue of files passed\n");
        exit(EXIT_FAILURE);
    }

    //connect to the collector, if fails exit
    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("error connecting to socket in masterworker");
        exit(EXIT_FAILURE);
    }

    socketfd = socket_fd; //global variable socketfd = socket_fd

    //create threadpool
    threadpool = createThreadPool(nThreads, queueSize, delay, socket_fd);

    //get files from the directory or the queue of files
    getArgs(argc, argv, threadpool, &queue, path);

    //Wait for threads to finish, after all the tasks in the queue are executed
    destroyThreadPool(threadpool, 0);


    //send message "quit" to collector
    if(quit == 0){
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
    }

    //close the socket
    if(closeSocket(socket_fd) == 0){
        exit(EXIT_FAILURE);
    }

    //free directory path
    if(path != NULL){
        if(strlen(path) > 0){
            free(path);
        }
    }

    return NULL;
}

