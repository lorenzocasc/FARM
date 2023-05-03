//
// Created by Lorenzo Cascone on 02/05/23.
//

#include "headers/MasterWorker.h"
#include "headers/input_parser.h"
#include "headers/threadpool.h"
#include "headers/utils.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "headers/queue.h"


static long nThreads = 4;
static long queueSize = 8;
static long delay = 0;


void* ciao(char *string){
    printf("Dal thread %s\n", string);
    return NULL;
}


void *executeMasterWorker(int argc, char* argv[]) {

    char *path = malloc(sizeof(char) * 255);
    node_t *head = NULL;
    int queue=0; //flag that assumes 1 only if a queue of files is passed
    getConfigArgs(argc, argv, &nThreads, &queueSize, path, &delay, &queue);

    if(queue == 0 && strlen(path)==0){
        printf("Error: no path or queue of files passed\n");
        exit(EXIT_FAILURE);
    }

    printf("nThreads: %ld\n", nThreads);
    printf("queueSize: %ld\n", queueSize);
    printf("path: %s\n", path);
    printf("delay: %ld\n", delay);
    printf("queue: %d\n", queue);

    threadpool_t* threadpool = createThreadPool(nThreads, queueSize);

    //getConfigArgs(argc, argv, &nThreads, &queueSize, path, &delay);
    getArgs(argc, argv, threadpool, &queue, path, &head);


    //Wait for threads to finish
    destroyThreadPool(threadpool,0);

    //free memory
    free(path);
    freeQueue(&head);


    printf("threadpool->queue_size: %d\n", threadpool->queue_size);
    printf("threadpool->num_threads: %d\n", threadpool->numthreads);

}

