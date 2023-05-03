//
// Created by Lorenzo Cascone on 03/05/23.
//

#include "headers/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//push a new node in the queue
void push(node_t **head, char *string) {
    node_t *tmp = malloc(sizeof(node_t));

    if(tmp == NULL){
        perror("malloc error");
        exit(EXIT_FAILURE);
    }

    int len=strlen(string);
    tmp->string = malloc(sizeof(char)*len+1);

    if(tmp->string == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strncpy(tmp->string,string,len);
    tmp->string[len]='\0';
    tmp->next = *head;
    *head=tmp;
}


//print queue
void printQueue(node_t *head) {
    node_t *current = head;

    while (current != NULL) {
        printf("print queue %s\n", current->string);
        current = current->next;
    }
}

//free queue
void freeQueue(node_t **head) {
    while (*head != NULL) {
        node_t *tmp = *head;
        *head = (*head)->next;
        free(tmp->string);
        free(tmp);
    }
}