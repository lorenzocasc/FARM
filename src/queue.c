//
// Created by Lorenzo Cascone on 03/05/23.
//

#include "../headers/queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//push a new node in the queue
void push(node_t **head, char *string, long sum) {
    node_t *tmp = malloc(sizeof(node_t));
    if (tmp == NULL) {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }

    int len = strlen(string);
    tmp->path = malloc(sizeof(char) * len + 1);
    tmp->sum = sum;
    if (tmp->path == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strncpy(tmp->path, string, len);
    tmp->path[len] = '\0';
    tmp->next = *head;
    *head = tmp;
}

//push a new node in the queue, ordered by sum, smaller to bigger
void pushOrdered(node_t **head, char *string, long sum) {
    node_t *tmp = malloc(sizeof(node_t));
    if (tmp == NULL) {
        perror("malloc error");
        free(string);
        exit(EXIT_FAILURE);
    }

    int len = strlen(string);
    tmp->path = malloc(sizeof(char) * len + 1);
    tmp->sum = sum;
    if (tmp->path == NULL) {
        perror("malloc");
        free(string);
        exit(EXIT_FAILURE);
    }

    strncpy(tmp->path, string, len);
    tmp->path[len] = '\0';

    node_t *current;
    if (*head == NULL || (*head)->sum >= tmp->sum) {
        tmp->next = *head;
        *head = tmp;
    } else {
        current = *head;
        while (current->next != NULL && current->next->sum < tmp->sum) {
            current = current->next;
        }
        tmp->next = current->next;
        current->next = tmp;
    }

    free(string);
}


//print queue
void printQueue(node_t *head) {
    node_t *current = head;
    while (current != NULL) {
        //printf("------------------\n");
        //printf("Element %d : ", c);
        //printf("Sum-> %ld \n", current->sum);
        printf("%ld %s\n", current->sum, current->path);
        //printf("Path-> %s ", current->path);
        //printf("------------------\n");
        current = current->next;
    }
}

//ordinated insert in queue, smaller to bigger sum
void insert(node_t **head, char *string, long sum) {
    node_t *tmp = malloc(sizeof(node_t));
    if (tmp == NULL) {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }

    int len = strlen(string);
    tmp->path = malloc(sizeof(char) * len + 1);
    tmp->sum = sum;
    if (tmp->path == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    strncpy(tmp->path, string, len);
    tmp->path[len] = '\0';

    node_t *current;
    if (*head == NULL || (*head)->sum >= tmp->sum) {
        tmp->next = *head;
        *head = tmp;
    } else {
        current = *head;
        while (current->next != NULL && current->next->sum < tmp->sum) {
            current = current->next;
        }
        tmp->next = current->next;
        current->next = tmp;
    }
}



//free queue
void freeQueue(node_t **head) {
    while (*head != NULL) {
        node_t *tmp = *head;
        *head = (*head)->next;
        free(tmp->path);
        free(tmp);
    }
}