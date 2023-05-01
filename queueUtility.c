//
// Created by Lorenzo Cascone on 01/05/23.
//

#include "headers/queueUtility.h"
#include <string.h>
#include <errno.h>

void printList(node_t *head) {
    node_t *current = head;
    while (current != NULL) {
        printf("fd: %d, file: %s\n", current->fd, current->file);
        current = current->next;
    }
}

void push(node_t **head, int fd, char *file) {
    node_t *new_node;
    new_node = malloc(sizeof(node_t));
    if(new_node == NULL) {
        perror("malloc");
        exit(errno);
    }
    new_node->fd = fd;
    new_node->file = malloc(sizeof(char) * (strlen(file) + 1));
    if(new_node->file == NULL) {
        perror("malloc");
        exit(errno);
    }
    strcpy(new_node->file, file);
    new_node->next = *head;
    *head = new_node;
}