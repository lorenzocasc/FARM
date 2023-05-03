//
// Created by Lorenzo Cascone on 01/05/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include "headers/threadpool.h"
#include "headers/MasterWorker.h"
#include "headers/input_parser.h"
#include "headers/queue.h"

#define maxPath 255


long file_size(const char *path) {

    long size = 0;
    struct stat s;
    if (stat(path, &s) == 0)
        size = s.st_size;
    else {
        perror("stat");
        return -1;
    }

    return size;
}


int get_fd(const char *filename) {

    int fd;
    if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("open");
        exit(errno);
    }

    return fd;
}

char *get_file_extension(const char *filename) {

    char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}

//Checks if a file is binary or not
int is_binary(const char *filename) {

    char *extension = get_file_extension(filename);

    if (strncmp(extension, "dat", strlen(extension)) != 0)
        return 0;

    long filesize = file_size(filename);

    if ((filesize % 8) != 0)
        return 0;
    else
        return 1;
}

//Checks if a file path is regular or not
int is_regular(const char *path) {

    struct stat s;
    if (stat(path, &s) != 0)return 0;

    return 1;
}

//Checks if a file path is a directory or not
int isDirectory(const char *path) {

    struct stat s;
    if (stat(path, &s) != 0)return 0;

    return S_ISDIR(s.st_mode);
}

/*
//Function that fetches all the files in a directory and sends them to the threadpool
int directoryFetch(char *path) {

    DIR *dir;
    struct dirent *ent;
    int length;
    char newPath[maxPath];

    if ((dir = opendir(path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {

                length = strlen(path) + strlen(ent->d_name) + 2;
                strncpy(newPath, path, length);
                strncat(newPath, "/", length);
                strncat(newPath, ent->d_name, length);
                length = strlen(newPath);
                newPath[length] = '\0';

                if (isDirectory(newPath)) {
                    directoryFetch(newPath);
                } else {
                    if (isValid(newPath) == 1) {
                        printf("printf da directoryFetch %s\n", newPath);
                    }
                }
            }
        }
        closedir(dir);
    } else {
        perror("opendir");
        return 1;
    }
    return 0;
}
*/
int directoryFetch(char *path, threadpool_t *pool, node_t** node) {
    DIR *dir;
    struct dirent *ent;
    char stack[maxPath][maxPath];
    int stackSize = 0;

    if ((dir = opendir(path)) != NULL) {
        while (1) {
            if ((ent = readdir(dir)) != NULL) {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    char *newPath = malloc(maxPath * sizeof(char));
                    snprintf(newPath, maxPath, "%s/%s", path, ent->d_name);
                    if (isDirectory(newPath)) {
                        if (stackSize < maxPath) {
                            strncpy(stack[stackSize], newPath, maxPath);
                            stackSize++;
                        } else {
                            fprintf(stderr, "Stack overflow\n");
                            free(newPath);
                            return 0;
                        }
                    } else {
                        if (isValid(newPath) <= 0) {
                            printf("Error: invalid file path\n");
                            continue;
                        } else {
                            push(node, newPath);
                        }
                    }
                    free(newPath);
                }
            } else {
                if (stackSize > 0) {
                    stackSize--;
                    if ((dir = opendir(stack[stackSize])) != NULL) {
                        strncpy(path, stack[stackSize], maxPath);
                        continue;
                    } else {
                        perror("opendir");
                        return 1;
                    }
                } else {
                    break;
                }
            }
        }
        closedir(dir);
    } else {
        perror("opendir");
        return 1;
    }
    return 0;
}


//Checks if a file path exists, if so, checks if it's regular and binary
int isValid(const char *path) {

    struct stat s;
    int res = stat(path, &s);
    if (res == -1)return res;

    return (is_binary(path) && is_regular(path));

}

//Function that transforms a string to int with strtol and handles errors
int isNumber(const char *s, long *n) {
    if (s == NULL) return 1;
    if (strlen(s) == 0) return 1;
    char *e = NULL;
    errno = 0;
    long val = strtol(s, &e, 10);
    if (errno == ERANGE) return 2;    // overflow
    if (e != NULL && *e == (char) 0) {
        *n = val;
        return 0;   // successo
    }
    return 1;   // non e' un numero
}


void getConfigArgs(int argc, char *input[], long *nThreads, long *queueSize, char *path, long *delay, int *queue) {
    int opt;
    while (optind < argc) {
        if ((opt = getopt(argc, input, "n:q:d:t:")) != -1) {
            switch (opt) {
                case 'n':
                    if (isNumber(optarg, nThreads)) {
                        printf("Error on %c: invalid argument\n", optopt);
                        exit(1);
                    }
                    break;
                case 'q':
                    if (isNumber(optarg, queueSize)) {
                        printf("Error on %c: invalid argument\n", optopt);
                        exit(1);
                    }
                    break;
                case 'd':
                    strcpy(path, optarg);
                    break;
                case 't':
                    if (isNumber(optarg, delay)) {
                        printf("Error on %c: invalid argument\n", optopt);
                        exit(1);
                    }
                    break;
                case ':':
                    printf("Error: '-%c' missing argument\n", optopt);
                    exit(1);
                default:
                    printf("Error: invalid argument\n");
                    exit(1);
            }
        } else {
            *queue = 1;
            optind++;
        }
    }
}


void getArgs(int argc, char *input[], threadpool_t *pool, int *queue, char *path, node_t **head) {
    int opt;
    optind = 1;

    if (strlen(path) != 0) {
        directoryFetch(path, pool,head);
    }

    node_t *temp = *head;
    //iterate over the list of nodes and add the tasks to the threadpool
    while (temp != NULL) {
        int x = addToThreadPool(pool, (void *) ciao, temp->string);
        if (x == -1) {
            printf("Error: Error while inserting the task \n");
            temp = temp->next;
            continue;
        }
        temp = temp->next;
    }

    if (*queue == 1) {
        while (optind < argc) {
            if ((opt = getopt(argc, input, "n:q:d:t:")) == -1) {
                if (isValid(input[optind]) <= 0) {
                    printf("Error: invalid file path\n");
                    optind++;
                    continue;
                } else {
                    int x = addToThreadPool(pool, (void *) ciao, input[optind]);
                    if (x == -1) {
                        printf("Error: Error while inserting the task \n");
                        optind++;
                        continue;
                    }
                    optind++;
                }
            }
        }
    }

}

