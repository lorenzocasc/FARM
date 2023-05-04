//
// Created by Lorenzo Cascone on 01/05/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/input_parser.h"
#include "headers/MasterWorker.h"
#include "headers/Collector.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>




int main(int argc, char *argv[]) {

    //Check if the number of arguments is correct
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <-n value> <-q value> <-d value> <-t value> <file-list>\n", argv[0]);
        exit(EXIT_FAILURE);
    }



    pid_t pid = fork();
    if(pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if(pid == 0){
        collectorExecutor();
    }
    if(pid > 0){
        sleep(1);
       executeMasterWorker(argc, argv);
    }







}