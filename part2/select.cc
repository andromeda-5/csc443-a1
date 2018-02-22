#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <iostream>

#include "library.h"

/**
 * Select single attribute with parameters <start> and <end>
 * <attribute_id> is the index of the attribute to be selected
 * (e.g. 0 for the first attribute, 1 for the second attribute, etc.)
 */
int main(int argc, char **argv) {
	if (argc != 6) {
		printf("Usage: <heapfile> <attribute_id> <start> <end> <page_size>\n");
		return 1;
	}

	FILE *heap_ptr = fopen(argv[1], "rb");
	if (!heap_ptr) {
		printf("Couldn't open file %s\n", argv[1]);
		return 1;
	}

	int attribute_id = strtol(argv[2], NULL, 10);
	char *start = argv[3], *end = argv[4];
	int page_size = strtol(argv[5], NULL, 10);
	int total_records = 0, selected_records = 0;

	Heapfile *heapfile = (Heapfile *)malloc(sizeof(Heapfile));
	heapfile->file_ptr = heap_ptr;
	heapfile->page_size = page_size;
	heapfile->slot_size = RECORD_SIZE;

	struct timeb s, e;
	ftime(&s);

	RecordIterator* rec_it = new RecordIterator(heapfile);

	while(rec_it->hasNext()) {

		Record r = rec_it->next();
		if (std::string(r.at(attribute_id)).compare(start) >= 0 && std::string(r.at(attribute_id)).compare(end) <= 0) {
			printf("%.5s\n", r.at(attribute_id));
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