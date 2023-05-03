//
// Created by Lorenzo Cascone on 03/05/23.
//

#ifndef SOLPROJECT_QUEUE_H
#define SOLPROJECT_QUEUE_H

//Linked queue
typedef struct node {
    char *string;
    struct node *next;
} node_t;

//push a new node in the queue
void push(node_t **head, char *string);
void printQueue(node_t *head);
void freeQueue(node_t **head);


#endif //SOLPROJECT_QUEUE_H
