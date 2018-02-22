#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "library.h"

// Insert all records in the CSV file to a heap file
// similarly as in the case with scan, the page size NEEDS to match the original page size
int main(int argc, char **argv) {
	if (argc != 4) {
		printf("Usage: <heapfile> <csv_file> <page_size>\n");
		return 1;
	}

	FILE *csv_ptr = fopen(argv[2], "r");
	if (!csv_ptr) {
    	printf("Couldn't open: %s\n", argv[2]);
    	return 1;
    }

	FILE *heap_ptr = fopen(argv[1], "rb+");
    if (!heap_ptr) {
    	printf("Couldn't open: %s\n", argv[1]);
    	return 1;
    }
    int page_size = strtol(argv[3], NULL, 10);
	
    //load all the records
	std::vector<Record*> records;
	load_records(csv_ptr, &records);
	fclose(csv_ptr);

	printf("loaded all records\n");
	if (records.size() == 0) {
		printf("No records found\n");
		return 1;
	}

	//asumming everything went well so far find the last directory
	Heapfile *heapfile = (Heapfile *)malloc(sizeof(Heapfile));
	heapfile->file_ptr = heap_ptr;
	heapfile->page_size = page_size;
	heapfile->slot_size = RECORD_SIZE;

	Page *page = (Page *)malloc(sizeof(Page));
	init_fixed_len_page(page, page_size, RECORD_SIZE);

	//find the very last directory
	printf("about to call the alloc_page function\n");
	PageID page_id = alloc_page(heapfile);
	printf("the page_id that's available %d\n", page_id);
	read_page(heapfile, page_id, page);
	for (int i = 0; i < records.size(); i++) {
		printf("this is the i in question %d\n", i);
		printf("about to call the add_fixed_len page function\n");
		int add_result = add_fixed_len_page(page, records.at(i));
		printf("just added record to page\n");
		if (add_result == -1){
			printf("the result is -1 so allocating new page\n");
			write_page(page, heapfile, page_id);
			page_id = alloc_page(heapfile);
			free(page->data);
			init_fixed_len_page(page, page_size, RECORD_SIZE);
			add_fixed_len_page(page, records.at(i));
		}
	}

	printf("writing the last page\n");
	write_page(page, heapfile, page_id);
	fclose(heap_ptr);
	free(page->data);
	free(page);

	return 0;
}