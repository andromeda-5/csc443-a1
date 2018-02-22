#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <iostream>
#include <algorithm>

#include "library.h"

/** 
 * Select a single attribute from the column store where parameter
 * <return_attribute_id> (B) is the index of an attribute to be projected and returned
 * <attribute_id> (A) is the index of (a possibly different) attribute whose value must
 * between <start> and <end>
 */
int main(int argc, char **argv) {
	if (argc != 7) {
		printf("Usage: <colstore_name> <attribute_id> <return_attribute_id> <start> <end> <page_size>\n");
		return 1;
	}

	std::string attr_filename = std::string(argv[1]) + "/" + std::string(argv[2]);
	const char *attr_tmp = attr_filename.c_str();
	FILE *heap_ptr = fopen(attr_tmp, "rb");
	if (!heap_ptr) {
		printf("Couldn't open %s\n", attr_tmp);
		return 1;
	}
	std::string return_filename = std::string(argv[1]) + "/" + std::string(argv[3]);
	const char *return_tmp = return_filename.c_str();

	//int attribute_id = strtol(argv[2], NULL, 10);
	//int return_attribute_id = strtol(argv[3], NULL, 10);
	char *start = argv[4], *end = argv[5];
	int page_size = strtol(argv[6], NULL, 10);

	int total_records = 0, selected_records = 0;

	Heapfile *heapfile = (Heapfile *)malloc(sizeof(Heapfile));
	heapfile->file_ptr = heap_ptr;
	heapfile->page_size = page_size;
	heapfile->slot_size = COL_TUP_SIZE;

	struct timeb s, e;
	ftime(&s);

	std::vector<int> selected_ids;

	RecordIterator* rec_it = new RecordIterator(heapfile);
	while(rec_it->hasNext()) {
		printf("has next!\n");

		Record r = rec_it->next();
		printf("id: %s, value %s\n", r.at(0), r.at(1));
		printf("%s %s\n", start, end);
		if (std::string(r.at(1)).compare(start) >= 0 && std::string(r.at(1)).compare(end) <= 0) {

			printf("%s %s\n", start, end);
			printf("FOUND A MATCH!\n");
			printf("puttin the value: %lu\n", strtol(r.at(0), NULL, 10));

			selected_ids.push_back(strtol(r.at(0), NULL, 10));
			selected_records++;


		}
		printf("No match FOUND\n");

		total_records++;
	}
	fclose(heap_ptr);
	free(rec_it);

	heap_ptr = fopen(return_tmp, "rb");
	if (!heap_ptr) {
		printf("Couldn't open %s\n", return_tmp);
		return 1;
	}
	heapfile->file_ptr = heap_ptr;
	printf("this is iterator 2\n");
	rec_it = new RecordIterator(heapfile);

	while(rec_it->hasNext()) {

		Record r = rec_it->next();
		if(std::find(selected_ids.begin(), selected_ids.end(), strtol(r.at(0), NULL, 10)) != selected_ids.end()) {
			printf("%.5s\n", r.at(1));
		}
	}

	ftime(&e);
	free(rec_it);
	printf("It took %lums to select %d record(s) from a total of %d records(s)\n", (e.time * 1000 + e.millitm) - (s.time * 1000 + s.millitm), selected_records, total_records);
	fclose(heap_ptr);
	free(heapfile);
	return 0;
}