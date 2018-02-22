#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "library.h"

//Print out all records in a heap file
//Note on this function: the page size HAS to match the page size with which the heap 
//was created 
int main(int argc, char **argv){
	if (argc != 3) {
		printf("Usage: <heapfile> <page_size>\n");
		return 1;
	}

	FILE *heap_ptr = fopen(argv[1], "rb");
    if (!heap_ptr) {
    	printf("Couldn't open: %s\n", argv[1]);
    	return 1;
    }
    int page_size = strtol(argv[2], NULL, 10);

	Heapfile heapfile;
	heapfile.file_ptr = heap_ptr;
	heapfile.page_size = page_size;
	heapfile.page_size = RECORD_SIZE;

	//iterate through the records
	RecordIterator *rec_it = new RecordIterator(&heapfile);
	while (rec_it->hasNext()) {
		Record r = rec_it->next();
		print_record(&r); 
	}

	fclose(heapfile.file_ptr);
	free(rec_it);

	return 0;
}