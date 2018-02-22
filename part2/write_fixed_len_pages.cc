#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/timeb.h>
#include <vector>

#include "library.h"

int main(int argc, char **argv){
	FILE *page_ptr, *csv_ptr;
	int page_size, num_pages = 0;
	struct timeb start, end;

	if (argc != 4) {
		printf("Usage: write_fixed_len_pages <csv_file> <page_file> <page_size>\n");
		return 1;
	}

	csv_ptr = fopen(argv[1], "r");
	if (!csv_ptr){
		printf("Couldn't open: %s\n", argv[1]);
		return 1;
	}

	page_ptr = fopen(argv[2], "w+b");
	if (!page_ptr) {
		printf("Couldn't open: %s\n", argv[2]);
		return 1;
	}

	page_size = strtol(argv[3], NULL, 10);

	//load all the records
	std::vector<Record*> records;
	load_records(csv_ptr, &records);
	fclose(csv_ptr);

	ftime(&start);

	Page* page = (Page *)malloc(sizeof(Page));
	init_fixed_len_page(page, page_size, RECORD_SIZE);
	num_pages++;

	//printf("The number of records is: %lu\n", records.size());
	//write records to page
	for (int i = 0; i < records.size(); i++) {
		//printf("Writing the %dth record\n", i);
		if (add_fixed_len_page(page, records.at(i)) == -1) {
			//printf("Not enough space, creating another page\n");
			// write the file
			fwrite(page->data, 1, page->page_size, page_ptr);
			fflush(page_ptr);

			//free data
			free(page->data);

			//create new page
			//printf("THIS IS THE PAGE SIZE: %d\n", page->page_size);
			init_fixed_len_page(page, page->page_size, RECORD_SIZE);
			add_fixed_len_page(page, records.at(i));
			//printf("THIS IS THE PAGE SIZE: %d\n", page->page_size);
			num_pages++;
		}

	}

	fwrite(page->data, 1, page->page_size, page_ptr);
	fflush(page_ptr);
	fclose(page_ptr);

	//clean up
	free(page->data);
	free(page);
	ftime(&end);

	printf("NUMBER OF RECORDS: %lu\n", records.size());
	printf("NUMBER OF PAGES: %d\n", num_pages);
	printf("TIME: %lu\n", (end.time * 1000 + end.millitm) - (start.time * 1000 + start.millitm));

	return 0;
}