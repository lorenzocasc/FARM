//
// Created by Lorenzo Cascone on 01/05/23.
//
#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <dirent.h>
#include "../headers/threadpool.h"
#include "../headers/MasterWorker.h"
#include "../headers/input_parser.h"
#include <getopt.h>
#include <unistd.h>

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


//Function that fetches all the files in a directory and sends them to the threadpool
int directoryFetch(char *path, threadpool_t *pool) {

    DIR *dir;
    struct dirent *ent;
    int length;
    char temp_array[maxPath];

    if ((dir = opendir(path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {

                length = strlen(path) + strlen(ent->d_name) + 2;
                strncpy(temp_array, path, length);
                strncat(temp_array, "/", length);
                strncat(temp_array, ent->d_name, length);
                length = strlen(temp_array);
                temp_array[length] = '\0';

                if (isDirectory(temp_array)) {
                    directoryFetch(temp_array, pool);
                } else {
                    if (isValid(temp_array) == 1) {
                        char *newPath = malloc(length * sizeof(char)+1);
                        strncpy(newPath, temp_array, length);
                        newPath[length] = '\0';
                        addToThreadPool(pool, (void *) value, newPath);
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


void getConfigArgs(int argc, char *input[], long *nThreads, long *queueSize, char **path, long *delay, int *queue) {
    int opt = 0;

    while ((opt = getopt(argc, input, "n:q:d:t:")) != -1) {
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
                *path = malloc(strlen(optarg) + 1);
                strncpy(*path, optarg, strlen(optarg));
                (*path)[strlen(optarg)] = '\0';
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
            case '?':
                printf("Error: invalid argument '-%c'\n", optopt);
                exit(1);
            default:
                printf("Error: invalid argument\n");
                exit(1);
        }
    }

    if (opt == -1 && (argc-optind)>0) {
        *queue = 1;
    }
    //}

}


void getArgs(int argc, char *input[], threadpool_t *pool, const int *queue, char *path) {
    int opt;
    //optind = 1;

    if (path != NULL) {
        if (strlen(path) != 0) {
            directoryFetch(path, pool);
        }
    }


    if (*queue == 1) {
        while (optind < argc) {
            if (isValid(input[optind]) <= 0) {
                printf("Error: invalid file path\n");
                optind++;
                continue;
            } else {
                char *pis = malloc(strlen(input[optind]) + 1);
                strcpy(pis, input[optind]);
                pis[strlen(input[optind])] = '\0';
                int x = addToThreadPool(pool, (void *) value, pis);
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

