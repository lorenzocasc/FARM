//
// Created by Lorenzo Cascone on 01/05/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/input_parser.h"
#include "headers/MasterWorker.h"


int main(int argc, char *argv[]) {

    //char* myArgs[] = {"program_name", "-n", "5","ciao.dat","cane","gatto", "-q", "6", "-d", "iv.txt", "-t", "300", NULL};
    //argc = sizeof(myArgs) / sizeof(myArgs[0]) - 1;


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