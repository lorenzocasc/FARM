CC = gcc

CFLAGS = -g -Wall -std=c99 -I ./headers -pthread

CFLAGSFARM = -g -Wall -std=c99

directory = src

obj = $(directory)/input_parser.o $(directory)/Collector.o $(directory)/queue.o $(directory)/threadpool.o $(directory)/MasterWorker.o $(directory)/main.o

objgenerafile = $(directory)/generafile

EXE = $(directory)/farm

.PHONY : clean test


#to clean the directory from object files, txt files and binary files
clean :
	-rm -f $(obj) $(objgenerafile) $(directory)/file*.dat $(directory)/expected.txt $(EXE)
	-rm -f -r $(directory)/testdir

#to test the program
test :$(obj) $(EXE) $(objgenerafile)
	cd src; ./script_test.sh

#test :$(obj) $(EXE) $(objgenerafile)
#	cd src; ./test.sh


#to compile the program
compila : $(obj) $(EXE) $(objgenerafile);

$(EXE): $(obj)
	$(CC) $(CFLAGSFARM) $(obj) -o src/farm

generafile : $(directory)/generafile.c
	$(CC) $(CFLAGSFARM) -c generafile.c -o generafile

main.o : $(directory)/main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

queue.o: $(directory)/queue.c
	$(CC) $(CFLAGS) -c queue.c -o queue.o

input_parser.o : $(directory)/input_parser.c
	$(CC) $(CFLAGS) -c input_parser.c -o input_parser.o

threadpool.o : $(directory)/threadpool.c
	$(CC) $(CFLAGS) -c threadpool.c -o threadpool.o

Collector.o : $(directory)/Collector.c
	$(CC) $(CFLAGS) -c Collector.c -o Collector.o

MasterWorker.o : $(directory)/MasterWorker.c
	$(CC) $(CFLAGS) -c MasterWorker.c -o MasterWorker.o

