//
// Created by Lorenzo Cascone on 01/05/23.
//
#include <stdio.h>
#include <stdlib.h>



#ifndef SOLPROJECT_QUEUEUTILITY_H
#define SOLPROJECT_QUEUEUTILITY_H

typedef struct node {
    int fd;
    char *file;
    struct node *next;
} node_t;


void printList(node_t *head);
void push(node_t **head, int fd, char *file);
void freeList(node_t *head);


#endif //SOLPROJECT_QUEUEUTILITY_H
