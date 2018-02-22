CC = g++
CF = ./create_random_file
TOTAL_BYTES = 104857600

all: library.o create_random_file get_histogram mybigfile

library.o: library.cc library.h
	$(CC) -o library.o -c library.cc

create_random_file: create_random_file.cc library.o
	$(CC) -o $@ $< library.o 

get_histogram: get_histogram.cc
	$(CC) -o $@ $<

mybigfile: 
	$(CF) "mybigfile.txt" $(TOTAL_BYTES) $(TOTAL_BYTES)

clean: 
	$(RM) -r library.o create_random_file rf_* get_histogram mybigfile.txt