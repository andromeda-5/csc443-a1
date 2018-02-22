#include <stdio.h>
#include <sys/timeb.h>

#include "library.h"

// Build heap file from CSV file
int main(int argc, char **argv) {
	
	if (argc != 4) {
		printf("Usage: <csv_file> <heapfile> <page_size>\n");
		return 1;
	}

	FILE *csv_ptr, *heap_ptr;
	struct timeb start, end;
	
	
	csv_ptr = fopen(argv[1], "r");
	if (!csv_ptr){
		printf("Couldn't open: %s\n", argv[1]);
		return 1;
	}

    heap_ptr = fopen(argv[2], "w+b");
    if (!heap_ptr) {
    	printf("Coudln't open: %s\n", argv[2]);
    	return 1;
    }

	int page_size = strtol(argv[3], NULL, 10);

	//load all the records
	std::vector<Record*> records;
	load_records(csv_ptr, &records);
	fclose(csv_ptr);

	//printf("loaded all records\n");
	if (records.size() == 0) {
		printf("No records found\n");
		return 1;
	}


	ftime(&start);
	Heapfile *heapfile = (Heapfile *)malloc(sizeof(Heapfile));
	init_heapfile(heapfile, page_size, heap_ptr);
	heapfile->slot_size = RECORD_SIZE;
	// printf("initialized the heap\n");
	//currently the heapfile only contains 1 page for the directory 
	//so we need to make a new page 
	Page *page = (Page *)malloc(sizeof(Page));
	init_fixed_len_page(page, page_size, RECORD_SIZE);

	//add all records + write pages to heap
	PageID page_id = alloc_page(heapfile);
	for (int i = 0; i < records.size(); i++) {
		print_record(records.at(i));

		int add_result = add_fixed_len_page(page, records.at(i));
		// printf("just added record to page\n");
		if (add_result == -1){
			// printf("couldn't add a record, allocating\n");
			// this means that the page is full and we need to write it to the heap
			write_page(page, heapfile, page_id);
			// printf("wrote page into heap\n");
			// now we need to allocate a new page
			// if (page_id == 0) {
			// 	page_id = 1;
			// }
			page_id = alloc_page(heapfile);
			// printf("allocating page, page_id is %d\n", page_id);
			//clear the page again
			free(page->data);
			// printf("freed the data from page\n");
			init_fixed_len_page(page, page_size, RECORD_SIZE);
			// printf("initialized fixed len page\n");
			//read_page(heapfile, page_id, page); // load the empty file
			// printf("adding record to the page\n");
			add_fixed_len_page(page, records.at(i));
			// printf("added fixed len page\n");
		}
		//printf("written the %dth record\n", i);
	}

	write_page(page, heapfile, page_id);
	int next_dir, slot_1, free_1, slot_2, free_2, slot_3, free_3;
	// fread(&next_dir, sizeof(int), 1, heapfile->file_ptr);
	// fread(&slot_1, sizeof(int), 1, heapfile->file_ptr);
	// fread(&free_1, sizeof(int), 1, heapfile->file_ptr);
	// fread(&slot_2, sizeof(int), 1, heapfile->file_ptr);
	// fread(&free_2, sizeof(int), 1, heapfile->file_ptr);
	// fread(&slot_3, sizeof(int), 1, heapfile->file_ptr);
	// fread(&free_3, sizeof(int), 1, heapfile->file_ptr);

	// printf("size of int is %lu, size of char is %lu\n", sizeof(int), sizeof(char));

	// printf("final: %d %d %d %d %d %d %d\n", next_dir, slot_1, free_1, slot_2, free_2, slot_3, free_3);

	fclose(heapfile->file_ptr);
	ftime(&end);

	printf("TIME: %lums\n", (end.time * 1000 + end.millitm) - (start.time * 1000 + start.millitm));
	
	free(page->data);
	free(page);
	return 0;
}