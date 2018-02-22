#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>

#include "library.h"

/** Build a column store from CSV file
 * <colstore_name> should be a file directory to store the heap file
 */
int main(int argc, char** argv) {
	if (argc != 4) {
		printf("Usage: <csv_file> <colstore_name> <page_size>");
		return 1;
	}

	FILE *csv_ptr = fopen(argv[1], "r");
	if (!csv_ptr) {
		printf("Couldn't open %s\n", argv[1]);
		return 1;
	}

	int page_size = strtol(argv[3], NULL, 10);
	struct timeb start, end;

	//make the directory to store heaps
	int status = mkdir(argv[2], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status == -1) {
		printf("Error creating directory %s\n", argv[2]);
		return 1;
	}

	//load all the records
	std::vector<Record*> records;
	load_records(csv_ptr, &records);
	fclose(csv_ptr);

//	printf("loaded all records\n");
	if (records.size() == 0) {
		printf("No records found\n");
		return 1;
	}

	//printf("num of records found is %lu\n", records.size());

	std::string path = std::string(argv[2]);
	std::string filename = path + "/";
	ftime(&start);

	FILE *attribute_file;
	Heapfile *heapfile = (Heapfile *) malloc(sizeof(Heapfile));
	Page *page = (Page *) malloc(sizeof(Page));
	init_fixed_len_page(page, page_size, COL_TUP_SIZE);
	int page_capacity = fixed_len_page_capacity(page);
	for (int i = 0; i < NUM_ATTR; i++){
		//create a heapfile for that attribute_id
		const char *tmp = (filename + std::to_string(i)).c_str();
		//printf("filename %s\n", tmp);
		attribute_file = fopen(tmp, "wb+");
		if (!attribute_file) {
		//	printf("Couldn't open %s\n", filename.c_str());
			free(heapfile);
			return 1;
		}

		init_heapfile(heapfile, page_size, attribute_file);
		heapfile->slot_size = COL_TUP_SIZE;
		PageID pid = alloc_page(heapfile);
		init_fixed_len_page(page, page_size, COL_TUP_SIZE);
		//read_page(heapfile, pid, page);

		//add the attribute of every record to the heap
		for (int j = 0; j < records.size(); j++) {
		//	printf("the size of records is: %lu\n", records.size());
			//find an available slot
			Record *r = new Record;
			r->push_back(std::to_string(j).c_str());
			r->push_back((records.at(j))->at(i));
		//	printf("This is what we are going to insert into the page: %s %s\n", std::to_string(j).c_str(), (records.at(j))->at(i));

			int add_result = add_fixed_len_page_col(page, r);
			if (add_result == -1) {
				write_page(page, heapfile, pid);
				pid = alloc_page(heapfile);
				free(page->data);
				init_fixed_len_page(page, page_size, COL_TUP_SIZE);
				add_fixed_len_page_col(page, r);
			}

		//	printf("written the %dth record\n", j);
		}
		write_page(page, heapfile, pid);
		fclose(attribute_file);
	}
	ftime(&end);

	free(page->data);
	free(page);
	free(heapfile);
	printf("It took: %lums\n", (end.time * 1000 + end.millitm) - (start.time * 1000 + start.millitm));
	return 0;
}