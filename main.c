//
// Created by Lorenzo Cascone on 01/05/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/input_parser.h"


int main(int argc, char *argv[]) {

    //char* myArgs[] = {"program_name", "-n", "5","ciao","cane","gatto", "-q", "6", "-d", "iv.txt", "-t", "300", NULL};
    //argc = sizeof(myArgs) / sizeof(myArgs[0]) - 1;


    long *nThreads = malloc(sizeof(long));
    long *queueSize = malloc(sizeof(long));
    char *path = malloc(sizeof(char) * 256);
    long *delay = malloc(sizeof(long));
    node_t *head = NULL;

    getArgs(argc, argv, nThreads, queueSize, path, delay, &head);

    printList(head);
    printf("nThreads: %ld\n", *nThreads);
    printf("queueSize: %ld\n", *queueSize);
    printf("path: %s\n", path);
    printf("delay: %ld\n", *delay);


}