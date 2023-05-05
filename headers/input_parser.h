//
// Created by Lorenzo Cascone on 01/05/23.
//

#ifndef SOLPROJECT_PARSER_H
#define SOLPROJECT_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "threadpool.h"
#include "MasterWorker.h"

long file_size(const char *path);

int get_fd(const char *filename);

char *get_file_extension(const char *filename);

int is_binary(const char *filename);

void getConfigArgs(int argc, char *argv[], long *nThreads, long *queueSize, char **path, long *delay, int *queue);

void getArgs(int argc, char *argv[], threadpool_t *pool, const int *queue, char *path);

int isNumber(const char *str, long *num);

int is_regular(const char *path);

int isValid(const char *path);

int directoryFetch(char *path, threadpool_t *pool);

int isDirectory(const char *path);

#endif //SOLPROJECT_PARSER_H

