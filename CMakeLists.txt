cmake_minimum_required(VERSION 3.25)
project(solProject C)

set(CMAKE_C_STANDARD 11)

add_executable(solProject
        main.c headers/input_parser.h input_parser.c queueUtility.c)
