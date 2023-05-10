//
// Created by Lorenzo Cascone on 03/05/23.
//

#ifndef SOLPROJECT_QUEUE_H
#define SOLPROJECT_QUEUE_H
//Linked queue
typedef struct node {
    char *path;
    long sum;
    struct node *next;
} node_t;
void printQueue(node_t *head); //print queue
void freeQueue(node_t **head); //free queue
void pushOrdered(node_t **head, char *string,long sum); //push a new node in the queue, ordered by sum, smaller to bigger
#endif //SOLPROJECT_QUEUE_H
