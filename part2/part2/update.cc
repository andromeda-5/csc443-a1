#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "library.h"

//update one attribute of a single record in the heap file its record ID
//<attribute_id> is the index of the attribute to be updated (e.g. 0 for the first attribute,
//1 for the second attribute, etc)
//<new_value> will have the same fixed length (10 bytes)
int main(int argc, char **argv) {
	if (argc != 6) {
		printf("Usage: <heapfile> <record_id> <attribute_id> <new_value> <page_size>\n");
		return 1;
	}

	FILE *heap_ptr = fopen(argv[1], "rb+");
    if (!heap_ptr) {
    	printf("Couldn't open: %s\n", argv[1]);
    	return 1;
    }

    char *rid = argv[2];
    int attribute_id = strtol(argv[3], NULL, 10);
    char *new_value = argv[4];
    int page_size = strtol(argv[5], NULL, 10);

    Heapfile heapfile;
    heapfile.file_ptr = heap_ptr;
    heapfile.page_size = page_size;
    heapfile.slot_size = RECORD_SIZE;

    Page page;
    init_fixed_len_page(&page, page_size, RECORD_SIZE);

    //find the correct page to update
    //record id is page_slot
    PageID pid;
    int sid;
    parse_rid(rid, &pid, &sid);
    // check pid
    printf("pid %d, sid %d\n", pid, sid);
    if (!is_page_id_valid(&heapfile, pid)) {
    	printf("Invalid page_id\n");
    	return 1;
    }
    read_page(&heapfile, pid, &page);

    //check that the slot is occupied
    if (is_slot_occupied(&page, sid)) {
    	Record *r = new Record;
    	read_fixed_len_page(&page, sid, r);
    	(* r)[attribute_id] = new_value;
    	write_fixed_len_page(&page, sid, r);
    	write_page(&page, &heapfile, pid);
    } else {
    	printf("Slot is empty, can't update\n");
    }

    fclose(heap_ptr);
    return 0;
}