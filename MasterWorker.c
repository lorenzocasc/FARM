//
// Created by Lorenzo Cascone on 02/05/23.
//
#include <sys/socket.h>
#include "headers/MasterWorker.h"
#include "headers/input_parser.h"
#include "headers/threadpool.h"
#include "headers/utils.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/un.h>

#define SOCK_PATH "./farm.sck"

static long nThreads = 4;
static long queueSize = 8;
static long delay = 0;


long value(char *string){
    long sum = 0;
    long number;
    int i = 0;
    FILE *fp = fopen(string, "rb");
    if (fp == NULL){
        perror("fopen");
        free(string);
        exit(EXIT_FAILURE);
    }

    while (fread(&number, sizeof(number), 1, fp) == 1){
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


void *executeMasterWorker(int argc, char* argv[]) {
    struct sockaddr_un server_addr;
    int socket_fd;

    if((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("error creating socket in masterworker");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_PATH);

    char *path = malloc(sizeof(char) * 255);
    int fd_c;

    int queue=0; //flag that assumes 1 only if a queue of files is passed
    getConfigArgs(argc, argv, &nThreads, &queueSize, path, &delay, &queue);
    if(queue == 0 && strlen(path)==0){
        printf("Error: no directory or queue of files passed\n");
        exit(EXIT_FAILURE);
    }



    printf("--------------------\n");
    printf("nThreads: %ld\n", nThreads);
    printf("queueSize: %ld\n", queueSize);
    printf("path: %s\n", path);
    printf("delay: %ld\n", delay);
    printf("queue: %d\n", queue);
    printf("--------------------\n");


    //connect to socket
    if(connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        perror("error connecting to socket in masterworker");
        exit(EXIT_FAILURE);
    }


    //create threadpool
    threadpool_t* threadpool = createThreadPool(nThreads, queueSize, delay, socket_fd);

    //get files from the directory or the queue of files
    getArgs(argc, argv, threadpool, &queue, path);


    //Wait for threads to finish
    destroyThreadPool(threadpool,0);

    close(socket_fd);
    //free memory
    free(path);


    //printf("threadpool->queue_size: %d\n", threadpool->queue_size);
    //printf("threadpool->num_threads: %d\n", threadpool->numthreads);
    return NULL;
}


