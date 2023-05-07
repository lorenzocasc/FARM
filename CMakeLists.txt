cmake_minimum_required(VERSION 3.25)
project(solProject C)

set(CMAKE_C_STANDARD 11)

add_executable(solProject
        src/main.c headers/input_parser.h src/input_parser.c headers/threadpool.h headers/MasterWorker.h src/MasterWorker.c src/threadpool.c headers/utils.h headers/Collector.h src/Collector.c src/queue.c)
