//
// Created by Lorenzo Cascone on 01/05/23.
//

#ifndef SOLPROJECT_PARSER_H
#define SOLPROJECT_PARSER_H
#endif //SOLPROJECT_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

long file_size(const char *path);
int get_fd(const char *filename);
char *get_file_extension(const char *filename);
int is_binary(const char *filename);
void getArgs(int argc, char *argv[], long *nThreads, long *queueSize, char *path, long *delay);
int isNumber(char *str, long* num);
int is_regular(const char *path);
int isValid(const char *path);



