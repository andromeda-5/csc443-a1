#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <bitset>
#include <iostream>

#include "library.h"

//delete a single record in the heap file given its record id
int main(int argc, char **argv) {
	if (argc != 4) {
		printf("Usage: <heapfile> <record_id> <page_size>\n");
		return 1;
	}

	FILE *heap_ptr = fopen(argv[1], "rb+");
    if (!heap_ptr) {
    	printf("Couldn't open: %s\n", argv[1]);
    	return 1;
    }

    char *rid = argv[2];
    int page_size = strtol(argv[3], NULL, 10);

    Heapfile *heapfile = (Heapfile *)malloc(sizeof(Heapfile));
    heapfile->file_ptr = heap_ptr;
    heapfile->page_size = page_size;
    heapfile->slot_size = RECORD_SIZE;

    Page *page = (Page *)malloc(sizeof(Page));
    init_fixed_len_page(page, page_size, RECORD_SIZE);
    

    PageID pid;
    int sid;
    parse_rid(rid, &pid, &sid);

    printf("pid %d, sid %d\n", pid, sid);
    if (!is_page_id_valid(heapfile, pid)) {
    	printf("Invalid page_id\n");
    	return 1;
    }

    read_page(heapfile, pid, page);
	if (is_slot_occupied(page, sid)) {
		int slot_dir = floor(sid/8);
		int remainder = sid % 8;

		printf("slot_dir: %d, remainder %d\n", slot_dir, remainder);

		char slots;
		int offset = page->page_size - MIN_DIR_SIZE - (slot_dir * 8);
		memcpy(&slots, (char *)page->data + offset, sizeof(char));
		std::bitset<8> y(slots);
		std::cout << y;
		printf("\n");
		slots &= ~(slot_array[7-remainder]);
		std::bitset<8> z(slots);
		std::cout << z;
		printf("\n");
		memcpy((char *)page->data + offset, &slots, sizeof(char));
    	write_page(page, heapfile, pid);

    } else {
    	printf("Slot is already empty\n");
    }

    fclose(heap_ptr);
	return 0;
}