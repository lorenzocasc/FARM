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
#include "headers/threadpool.h"


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


void getArgs(int argc, char *input[], long *nThreads, long *queueSize, char *path, long *delay) {
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
        }else{
            optind++;
        }
    }
}

