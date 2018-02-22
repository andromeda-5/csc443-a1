#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <iostream>

#include "library.h"

/** 
 * Select a single attribute from the column store where parameter
 * <attribute_id> is the index of the attribute to be project and returned 
 * (e.g. 0 for the first attribute, 1 for the second attribute, etc.)
 * the value of the attribute must be between <start> and <end>
 */
int main(int argc, char **argv) {
	if (argc != 6) {
		printf("Usage: <colstore_name> <attribute_id> <start> <end> <page_size>\n");
		return 1;
	}

	std::string filename = std::string(argv[1]) + "/" + std::string(argv[2]);
	const char *tmp = filename.c_str();
	FILE *heap_ptr = fopen(tmp, "rb");
	if (!heap_ptr) {
		printf("Couldn't open %s\n", tmp);
		return 1;
	}

	int attribute_id = strtol(argv[2], NULL, 10);
	char *start = argv[3], *end = argv[4];
	int page_size = strtol(argv[5], NULL, 10);

	int total_records = 0, selected_records = 0;

	Heapfile *heapfile = (Heapfile *)malloc(sizeof(Heapfile));
	heapfile->file_ptr = heap_ptr;
	heapfile->page_size = page_size;
	heapfile->slot_size = COL_TUP_SIZE;

	struct timeb s, e;
	ftime(&s);

	RecordIterator* rec_it = new RecordIterator(heapfile);

	while(rec_it->hasNext()) {
		//printf("has next!\n");

		Record r = rec_it->next();
		//printf("id: %s, value %s\n", r.at(0), r.at(1));
		if (std::string(r.at(1)).compare(start) >= 0 && std::string(r.at(1)).compare(end) <= 0) {
			printf("%.5s\n", r.at(1));
			selected_records++;
		}

		total_records++;
	}

	ftime(&e);

	printf("It took %lums to select %d record(s) from a total of %d records(s)\n", (e.time * 1000 + e.millitm) - (s.time * 1000 + s.millitm), selected_records, total_records);
	fclose(heap_ptr);
	free(heapfile);
	free(rec_it);
	return 0;
}