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

static long nThreads = 4;
static long queueSize = 8;
static long delay = 0;


void* ciao(char *string){
    printf("Dal thread %s\n", string);
    return NULL;
}


void *executeMasterWorker(int argc, char* argv[]) {

    char *path = malloc(sizeof(char) * 255);
    int opt;
    getArgs(argc, argv, &nThreads, &queueSize, path, &delay);

    printf("nThreads: %ld\n", nThreads);
    printf("queueSize: %ld\n", queueSize);
    printf("path: %s\n", path);
    printf("delay: %ld\n", delay);

    threadpool_t* threadpool = createThreadPool(nThreads, queueSize);



    //This function retrives, if present, the list of files from argv

    optind = 1;
    while(optind < argc){
        if ((opt = getopt(argc, argv, "n:q:d:t:")) == -1){
            if(isValid(argv[optind]) <= 0){
                printf("Error: invalid file path %s\n", argv[optind]);
                optind++;
                continue;
            }else{
                int success = addToThreadPool(threadpool, (void*)ciao, argv[optind]);
                if(success == -1){
                    printf("Error: addToThreadPool\n");
                    optind++;
                    continue;
                }
                if(success == 1)continue;

                optind++;
            }
       }
    }

    //Wait for threads to finish
    destroyThreadPool(threadpool,0);



    //This function retrives, if present, the list of files from the directory and put them in the threadpool
    if(path!=NULL && strlen(path)>0){

    }else{
        free(path);
    }




    printf("threadpool->queue_size: %d\n", threadpool->queue_size);
    printf("threadpool->num_threads: %d\n", threadpool->numthreads);

    /*taskfun_t *temp = threadpool->pending_queue;
    for(int i = 0;i < threadpool->count; i++){
        printf("%s\n", temp[i].arg);
    }
    */

}

