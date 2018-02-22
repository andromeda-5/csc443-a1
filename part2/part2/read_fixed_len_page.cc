#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>

#include "library.h"


int main(int argc, char** argv) {
	if (argc != 3) {
		printf("Usage: read_fixed_len_page <page_file> <page_size>\n");
		return 1;
	}

	FILE* file_ptr = fopen(argv[1], "rb");
	if (!file_ptr) {
		printf("Couldn't open %s\n", argv[1]);
		return 1;
	}

	int page_size = strtol(argv[2], NULL, 10);
	struct timeb start, end;
	int num_records = 0;
	ftime(&start);
	Page *page = new Page;
	
	while (!feof(file_ptr)) {
		init_fixed_len_page(page, page_size, RECORD_SIZE);
		if (fread(page->data, page->page_size, 1, file_ptr) == 0) {
			break;
		} 

		char *bit_array = (char *)page->data + page->page_size - MIN_DIR_SIZE; 
		//want the most number of slots the page can contain
		int capacity = fixed_len_page_capacity(page);
		int offset = 0;
		
		for (int i=0; i < capacity; i++) {
			char slots;
			memcpy(&slots, bit_array, sizeof(char));
			if ((slots & slot_array[7 - (i%8)]) == slot_array[7 - (i%8)]) {
				Record r;
				read_fixed_len_page(page, i, &r);
				num_records++;
				//print_record(&r);
			}

			if (i > 0 && i %8 == 0) {
				offset++;
				bit_array -= offset; 
			}
		}
	}

	ftime(&end);

	printf("NUMBER OF RECORDS: %d\n", num_records);
	printf("TIME: %lu\n", (end.time * 1000 + end.millitm) - (start.time * 1000 + start.millitm));

	free(page->data);
	free(page);
}