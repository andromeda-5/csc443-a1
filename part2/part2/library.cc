#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <bitset>
#include <iostream>

#include "library.h"

// ++++++++++++++++++++++++++ HELPER FUNCTIONS ++++++++++++++++++++++++++ //
char *get_capacity_offset(Page *page) {
	return ((char *)page->data + page->page_size - sizeof(int));
}

int get_group(int page_capacity) {
	return (int)floor((float)page_capacity/(float)8);
}

void load_records(FILE *csv_ptr, std::vector<Record*>* records){
	int i;
	while (!feof(csv_ptr)) {
		Record *r = new Record;

		for (i = 0; i < NUM_ATTR; i++) {
			char *attr = (char *)malloc(LEN_ATTR);
			if (fread(attr, LEN_ATTR, 1, csv_ptr) == 0)
				break;
			fgetc(csv_ptr);
			r->push_back(attr);
		}
		if (i == 100) {
			records->push_back(r);
		}
	}
}

void print_record(Record *r) {
	int i;
	for (i = 0; i < r->size() - 1; i++){
		printf("%.10s,", r->at(i));
	}
	printf("%.10s\n", r->at(i));
}

int entries_per_dir(int page_size) {
	return (int)floor((page_size - HEAP_PTR_SIZE) / DIR_ENTRY_SIZE);
}

int get_file_size(Heapfile *heapfile) {
	fseek(heapfile->file_ptr, 0, SEEK_END);
	return ftell(heapfile->file_ptr);
}

bool is_page_id_valid(Heapfile *heapfile, PageID pid) {
	//printf("in is_page_id_valid\n");
	int file_size = get_file_size(heapfile);
	int num_pages = file_size / heapfile->page_size;
	int entries = entries_per_dir(heapfile->page_size);
	int page_size = heapfile->page_size;

	//printf("file_size: %d, num_pages %d, entries %d, page_size %d\n", file_size, num_pages, entries, page_size);

	int dirs = floor(pid/entries);
	int entry = pid % entries + 1;
	//printf("pid %d dirs %d, entry %d page_size %d entries %d\n", pid, dirs, entry, page_size, entries);

	int bytes_needed = (dirs * page_size * entries) + ((dirs + 1) * page_size) + (entry * page_size);

	//printf("file_size:  %d but bytes_needed is %d\n", file_size, bytes_needed);
	if (file_size < bytes_needed) 
		return false;
	return true;
}

void parse_rid(char *rid, PageID *pid, int *sid) {
	char *page = strtok(rid, "_");
	char *slot = strtok(NULL, "_");
	*pid = strtol(page, NULL, 10);
	*sid = strtol(slot, NULL, 10);
}

bool is_slot_occupied(Page *page, int slot) {

	int page_capacity = 0;

	//memcpy((char *)page->data + page->page_size - sizeof(int), &page_capacity, sizeof(int));
	memcpy(&page_capacity, (char *)page->data + page->page_size, sizeof(int));

	char first_byte;
	memcpy(&first_byte, (char *)page->data + RECORD_SIZE, sizeof(char));

	char kk;
	//memcpy((char *)page->data + page->page_size - sizeof(int) - sizeof(char), &kk, sizeof(char));
	//memcpy(&kk, (char *)page->data + page->page_size - sizeof(int) - sizeof(char), sizeof(char));
	memcpy(&kk, (char *)page->data + page->page_size - sizeof(int) - sizeof(char), sizeof(char));

	int slot_dir = floor(slot/8);
	int remainder = slot % 8;

	char slots;
	int offset = page->page_size - MIN_DIR_SIZE - (slot_dir * 8);
	memcpy(&slots, (char *)page->data + offset, sizeof(char));
	if ((slots & slot_array[7-remainder]) == slot_array[7-remainder]) {
		return true;
	}
	return false;
}

// ++++++++++++++++++++++++++ REQUIRED FUNCTIONS ++++++++++++++++++++++++++ //
int fixed_len_sizeof(Record *record) {
	return RECORD_SIZE;
}

void fixed_len_write(Record *record, void *buf) {
	for (int i = 0; i < record->size(); i++) {
		memcpy((char *)buf + i*LEN_ATTR, record->at(i), LEN_ATTR);
	}
}

void fixed_len_write_col(Record *record, void *buf) {
	memcpy((char *)buf, record->at(0), sizeof(int));
	memcpy((char *)buf + sizeof(int), record->at(1), LEN_ATTR);
}

void fixed_len_read(void *buf, int size, Record *record) {
	for (int i = 0; i < NUM_ATTR; i++) {
		V attribute = (char *)buf + i*LEN_ATTR;
		record->push_back(attribute);
	}
}

void fixed_len_read_col(void *buf, int size, Record *record) {

	V attribute1 = (char *)buf;
	V attribute2 = (char *)buf + sizeof(int);
	record->push_back(attribute1);
	record->push_back(attribute2);

}

int fixed_len_page_capacity(Page *page) {
	//printf("in fixed_len_page_capacity\n"); 
	int page_size_for_slots = page->page_size - sizeof(int); //2 bytes to store the int
	//printf("page_size: %d, slot_size: %d\n", page->page_size, page->slot_size);
	int max_slots = (int) floor(page_size_for_slots / page->slot_size); 
	//this is also the number of bits we need to represent if the slot is occupied or not
	int bytes_for_dir = ceil((float)max_slots / (float)8);
	page_size_for_slots -= bytes_for_dir;
	max_slots = (int) floor(page_size_for_slots / page->slot_size); 
	//printf("%d %d %d %d\n", page_size_for_slots, max_slots, bytes_for_dir, max_slots);
	return max_slots;
}

void init_fixed_len_page(Page *page, int page_size, int slot_size) {
	page->page_size = page_size;
	page->slot_size = slot_size;
	page->data = malloc(page_size);
	memset(page->data, 0, page->page_size);

	int capacity = fixed_len_page_capacity(page);
	//store the page capacity at the end
	memcpy(get_capacity_offset(page), &capacity, sizeof(int));
}	

int fixed_len_page_freeslots(Page *page){
	// going to store the information at the beginning
	//printf("Calculating number of free slots\n");
	int page_capacity = fixed_len_page_capacity(page);
	//printf("page_capacity: %d\n", page_capacity);
	//the next 8 bits are going to contain info about the first 8 slots of records
	char *bit_array = (char *)page->data + page->page_size - MIN_DIR_SIZE;
	int groups = get_group(page_capacity);
	int remainder = page_capacity % 8;
	int free_slots = 0;
	int i, j;

	//printf("groups: %d, remainder: %d, free_slots: %d\n", groups, remainder, free_slots);

	char slots;
	for (i = 0; i < groups; i++) {
		memcpy(&slots, bit_array - i, sizeof(char));
		for (j = 0; j < 8; j++) {
			//printf("Considering slot %d\n", i*8 + j);
			//std::bitset<8> x(slots);
			//std::cout << x;
			//printf("\n");
			if ((slots & slot_array[7 - j]) == 0x00) {
				free_slots++;
				//printf("free slots: %d\n", free_slots);
			}
		}
	}
	//process the remainder
	memcpy(&slots, bit_array - i, sizeof(char));
	for (i=7; i > 7-remainder; i--) {
		if ((slots & slot_array[i]) == 0x00) {
			//printf("Considering slot %d\n", groups*8 + 7-i);
			//std::bitset<8> x(slots);
			//std::cout << x;
			//printf("\n");
			free_slots++;
			//printf("free slots: %d\n", free_slots);
		}
	}
    //printf("but the number of free slots is: %d\n", free_slots);
	return free_slots;
}


int add_fixed_len_page(Page *page, Record *r){
	//printf("in the add_fixed_len_page function\n");
	int offset = -1;
	//printf("about to call the space available funciton\n");
	int space_available = fixed_len_page_freeslots(page);
	//printf("The number of slots available is %d\n", space_available);

	if (space_available != 0) {
		//find the array
		//printf("We have spots available\n");
		int page_capacity = fixed_len_page_capacity(page);
		char *bit_array = (char *)page->data + page->page_size - MIN_DIR_SIZE;
		int groups = get_group(page_capacity);
		int remainder = page_capacity % 8;
		int i, j;
		//printf("cap: %d, groups: %d, remainder: %d\n", page_capacity, groups, remainder);

		char slots;

		for (i = 0; i < groups; i++) {
			//printf("i is %d\n", i);
			memcpy(&slots, bit_array - i, sizeof(char));
			//std::bitset<8> x(slots);
			//std::cout << x;
			for (j = 0; j < 8; j++) {
				if ((slots & slot_array[7 - j]) == 0) {
					//printf("slot %d is available\n", i*8 + j);
					offset = i*8 + j;
					//printf("WRITING TO SLOT %d\n", offset);
					fixed_len_write(r, ((char *)page->data) + offset*page->slot_size);			
					slots |= (1 << j);
					//std::bitset<8> x(slots);
					//std::cout << x;
					//printf("\n");
					memcpy(bit_array - i, &slots, sizeof(char));
					return offset;
				}
			}
			//printf("exiting\n");
		}
		//process the remaining slots
		//printf("This is the remainder: %d\n", remainder);
		memcpy(&slots, bit_array + groups, sizeof(char));
		int sanity_check;
		memcpy(&sanity_check, (char *)page->data + page->page_size - sizeof(int), sizeof(int));
		//printf("sanity check capacity #1 %d\n", sanity_check);
		for (j=7; j > 7-remainder; j--) {
		//	printf("Processing the %dth record\n", 7-j);
			if ((slots & slot_array[j]) == 0x00) {
				int free_slots = fixed_len_page_freeslots(page);
			//	printf("and the free_slots is %d\n", free_slots);
				offset = groups * 8 + 7 - j;
			//	printf("WRITING TO SLOT %d\n", offset);
				memcpy(&sanity_check, (char *)page->data + page->page_size - sizeof(int), sizeof(int));
			//	printf("sanity check capacity #2 %d\n", sanity_check);
				fixed_len_write(r, (char *)page->data + offset*page->slot_size);
				slots |= (1 << (7 - j));
			//	std::bitset<8> x(slots);
			//	std::cout << x;
			//	printf("\n");
				memcpy(&sanity_check, (char *)page->data + page->page_size - sizeof(int), sizeof(int));
			//	printf("sanity check capacity #2 %d\n", sanity_check);
				memcpy(bit_array + groups, &slots, sizeof(char));
				free_slots = fixed_len_page_freeslots(page);
			//	printf("and the free_slots is %d\n", free_slots);
				memcpy(&sanity_check, (char *)page->data + page->page_size - sizeof(int), sizeof(int));
			//	printf("sanity check capacity #3 blah %d\n", sanity_check);
			//	std::bitset<8> y(slots);
			//	std::cout << y;
			//	printf("\n");
				return offset;
			}
		}
	} 

	return offset;
}

int add_fixed_len_page_col(Page *page, Record *r){
	//printf("in the add_fixed_len_page function\n");
	int offset = -1;
	//printf("about to call the space available funciton\n");
	int space_available = fixed_len_page_freeslots(page);
	//printf("The number of slots available is %d\n", space_available);

	if (space_available != 0) {
		//find the array
		//printf("We have spots available\n");
		int page_capacity = fixed_len_page_capacity(page);
		char *bit_array = (char *)page->data + page->page_size - MIN_DIR_SIZE;
		int groups = get_group(page_capacity);
		int remainder = page_capacity % 8;
		int i, j;
		//printf("cap: %d, groups: %d, remainder: %d\n", page_capacity, groups, remainder);

		char slots;

		for (i = 0; i < groups; i++) {
			//printf("i is %d\n", i);
			memcpy(&slots, bit_array - i, sizeof(char));
			//std::bitset<8> x(slots);
			//std::cout << x;
			for (j = 0; j < 8; j++) {
				if ((slots & slot_array[7 - j]) == 0) {
					//printf("slot %d is available\n", i*8 + j);
					offset = i*8 + j;
					//printf("WRITING TO SLOT %d\n", offset);
					fixed_len_write_col(r, ((char *)page->data) + offset*page->slot_size);			
					slots |= (1 << j);
			//		std::bitset<8> x(slots);
			//		std::cout << x;
			//		printf("\n");
					memcpy(bit_array - i, &slots, sizeof(char));
					return offset;
				}
			}
		//	printf("exiting\n");
		}
		//process the remaining slots
	//	printf("This is the remainder: %d\n", remainder);
		memcpy(&slots, bit_array + groups, sizeof(char));
		int sanity_check;
		memcpy(&sanity_check, (char *)page->data + page->page_size - sizeof(int), sizeof(int));
	//	printf("sanity check capacity #1 %d\n", sanity_check);
		for (j=7; j > 7-remainder; j--) {
	//		printf("Processing the %dth record\n", 7-j);
			if ((slots & slot_array[j]) == 0x00) {
				int free_slots = fixed_len_page_freeslots(page);
	//			printf("and the free_slots is %d\n", free_slots);
				offset = groups * 8 + 7 - j;
	//			printf("WRITING TO SLOT %d\n", offset);
				memcpy(&sanity_check, (char *)page->data + page->page_size - sizeof(int), sizeof(int));
	//			printf("sanity check capacity #2 %d\n", sanity_check);
				fixed_len_write_col(r, (char *)page->data + offset*page->slot_size);
				slots |= (1 << (7 - j));
	//			std::bitset<8> x(slots);
	//			std::cout << x;
	//			printf("\n");
				memcpy(&sanity_check, (char *)page->data + page->page_size - sizeof(int), sizeof(int));
	//			printf("sanity check capacity #2 %d\n", sanity_check);
				memcpy(bit_array + groups, &slots, sizeof(char));
				free_slots = fixed_len_page_freeslots(page);
	//			printf("and the free_slots is %d\n", free_slots);
				memcpy(&sanity_check, (char *)page->data + page->page_size - sizeof(int), sizeof(int));
	//			printf("sanity check capacity #3 %d\n", sanity_check);
				memcpy(&sanity_check, (char *)page->data + page->page_size - MIN_DIR_SIZE, sizeof(char));
	///			std::bitset<8> y(slots);
	//			std::cout << y;
	//			printf("\n");
				return offset;
			}
		}
	} 

	return offset;
}

void write_fixed_len_page(Page *page, int slot, Record *r){
	int capacity = fixed_len_page_capacity(page);
	if (slot > capacity) {
		printf("Slot is outside the permissible range\n");
		return;
	}

	//this is already assuming that the space is empty 
	char *record_offset = (char *)page->data + slot*page->slot_size;
	fixed_len_write(r, record_offset);

	//update the directory	
	int groups = get_group(capacity);
	int remainder = slot % 8;

	char *directory_char = get_capacity_offset(page) - groups;
	char slots;
	memcpy(&slot, directory_char, sizeof(char));
	slots |= (1 << (7-remainder));
	memcpy(directory_char, &slots, sizeof(char));
}


void read_fixed_len_page(Page *page, int slot, Record *r){
	int capacity = fixed_len_page_capacity(page);
	if (slot > capacity) {
		printf("Slot is outside the permissible range\n");
		return;
	}

	char* slot_ptr = (char *)page->data + (page->slot_size * slot);
	fixed_len_read(slot_ptr, page->slot_size, r);
}

void read_fixed_len_page_col(Page *page, int slot, Record *r){
	int capacity = fixed_len_page_capacity(page);
	if (slot > capacity) {
		printf("Slot is outside the permissible range\n");
		return;
	}

	char* slot_ptr = (char *)page->data + (page->slot_size * slot);
	fixed_len_read_col(slot_ptr, page->slot_size, r);
}

void init_dir(Heapfile *heapfile, PageID start) {
	//printf("initializing directory\n");
	int entries = entries_per_dir(heapfile->page_size);
	int page_size = heapfile->page_size;
	//printf("Entries per dir: %d", entries);

	//printf("going to loop through the entires: %d\n", entries);
	//all pages are empty at this mpoint
	for (int i=start; i< entries + start; i++) {
		fwrite(&i, sizeof(int), 1, heapfile->file_ptr);
		//fwrite(&page_size, sizeof(int), 1, heapfile->file_ptr);
		fwrite(&page_size, sizeof(int), 1, heapfile->file_ptr);
	}
	//rewind(heapfile->file_ptr);
}

void init_heapfile(Heapfile *heapfile, int page_size, FILE *file) {
	//printf("initializing the heapfile\n");
	heapfile->file_ptr = file;
	heapfile->page_size = page_size;

	//printf("writing next_heap as 0\n");
	//only one directory so far, no pointer to next dir
	int next_heap = 0; 
	fwrite(&next_heap, sizeof(int), 1, heapfile->file_ptr );

	//printf("initializing directory\n");
	init_dir(heapfile, 0);
	rewind(heapfile->file_ptr);
}

PageID alloc_page(Heapfile *heapfile) {
	//printf("in the alloc_page file \n");
	rewind(heapfile->file_ptr);
	int next_dir = 0;

	fread(&next_dir, sizeof(int), 1, heapfile->file_ptr);
	//printf("next_dir: %d\n", next_dir);

	int entries = entries_per_dir(heapfile->page_size);
	PageID curr_page_id = 0;

	int free_space = 0;
	int curr_dir = 0;
	PageID last_page_id = entries - 1;
	int page_size = heapfile->page_size;

	while (curr_page_id < last_page_id) {
		//printf("curr_page_id: %d, last_page_id %d\n", curr_page_id, last_page_id);
		fread(&curr_page_id, sizeof(int), 1, heapfile->file_ptr);
		fread(&free_space, sizeof(int), 1, heapfile->file_ptr);
		//printf("page_id: %d, free_space: %d\n", curr_page_id, free_space);

		if (free_space >= heapfile->slot_size) {
			//printf("found free space\n");
			rewind(heapfile->file_ptr);
			return curr_page_id;
		}

		//printf("checking: curr_page_id %d last_page_id - entries %d\n", curr_page_id, last_page_id - entries);
		if (curr_page_id == last_page_id - entries) {
			curr_dir++;
			curr_page_id++;
			// printf("curr_page was the same as last_page and cur dir : %d, %d, %d\n", curr_page_id, last_page_id, curr_dir);
			if (next_dir > 0) {
				// printf("next dir is greater than 0: %d\n", next_dir);
				fseek(heapfile->file_ptr, curr_dir*entries*page_size + curr_dir*page_size, SEEK_SET);
				fread(&next_dir, sizeof(int), 1, heapfile->file_ptr);
			} else {
				// printf("next dir is 0\n");
				fseek(heapfile->file_ptr, (curr_dir - 1)*entries*page_size + (curr_dir - 1)*page_size, SEEK_SET);
				fwrite(&curr_dir, sizeof(int), 1, heapfile->file_ptr);
				fseek(heapfile->file_ptr, curr_dir*entries*page_size + curr_dir*page_size, SEEK_SET);
				fwrite(&next_dir, sizeof(int), 1, heapfile->file_ptr);
				init_dir(heapfile, curr_page_id);
				fseek(heapfile->file_ptr, curr_dir*entries*page_size + curr_dir*page_size + sizeof(int), SEEK_SET);
			}
		}

		if (curr_page_id + 1 == last_page_id) {
			last_page_id  += entries;
		}
		curr_page_id++;
	}
	return -1; 
}



void read_page(Heapfile *heapfile, PageID pid, Page *page){
	//assume page has already been initialized
	//printf("in read_page function\n");
	//printf("pid: %d\n", pid);

	int directory = floor(pid / entries_per_dir(heapfile->page_size));
	int offset = heapfile->page_size * (directory + pid + 1);

	//printf("directory %d offset %d\n", directory, offset);

	fseek(heapfile->file_ptr, offset, SEEK_SET);
	fread(page->data, page->page_size, 1, heapfile->file_ptr);

	rewind(heapfile->file_ptr);
	//printf("exiting read page\n");
}

void write_page(Page *page, Heapfile *heapfile, PageID pid){
	//printf("in the write page function\n");
	
	//given the page id and the page we need to write to the heapfile
	int dirs = floor(pid/entries_per_dir(heapfile->page_size));
	int page_offset = (pid + dirs + 1) * (page->page_size); 
	//printf("Dirs: %d, page_offset: %d\n", dirs, page_offset);

	fseek(heapfile->file_ptr, page_offset, SEEK_SET);
	fwrite(page->data, page->page_size, 1, heapfile->file_ptr);

	//printf("wrote the page to the file\n");
	//update the directory entry
//	printf("going to update the directory\n");
	int dir_entry = pid % entries_per_dir(heapfile->page_size);
	int entry_offset =  HEAP_PTR_SIZE + 2 * sizeof(int) * dir_entry + sizeof(int); //skip the id
	int dir_offset = dirs * entries_per_dir(heapfile->page_size) * page->page_size + dirs*(page->page_size);
	
//	printf("pid %d dir_entry: %d, entry_offset: %d, dir_offset: %d\n", pid, dir_entry, entry_offset, dir_offset);
	fseek(heapfile->file_ptr, dir_offset + entry_offset , SEEK_SET);
	int free_space = fixed_len_page_freeslots(page) * page->slot_size;
//	printf("the new_free_space is %d page size is %d\n", free_space, page->page_size);
	fwrite(&free_space, sizeof(int), 1, heapfile->file_ptr);
	rewind(heapfile->file_ptr);
	fflush(heapfile->file_ptr);
}

RecordIterator::RecordIterator(Heapfile *heapfile) {
//	printf("in the iterator\n");
	this->heapfile = heapfile;
	this->page_id = 0;
	this->slot = 0;
	this->page = (Page *) malloc(sizeof(Page));


	init_fixed_len_page(this->page, (this->heapfile)->page_size, heapfile->slot_size);

	read_page(heapfile, this->page_id, this->page);	

}

RecordIterator::~RecordIterator() {
	rewind((this->heapfile)->file_ptr);
	free((this->page)->data);
	free(this->page);
}

bool RecordIterator::hasNext() {

	bool has_next = false;
	int capacity = fixed_len_page_capacity(this->page);
	int next_slot;
	PageID next_page;

	while (true) {
		// check current page first
		next_slot = this->slot;
		while (next_slot < capacity) {
			char *dir = (char *)((this->page)->data) + ((this->page)->page_size) - MIN_DIR_SIZE;
			int group = (int)floor(next_slot/8);
			int remainder = next_slot % 8;
			dir += group*sizeof(char);
			if ((*dir & slot_array[7-remainder]) != 0x00) {
				this->slot = next_slot;
				return true;
			}
			next_slot++;
		}

		assert(next_slot == capacity);
		//we should be at the end
		//we have exhausted a page 
		//is there another page?
		next_page = this->page_id + 1;
		if (is_page_id_valid(heapfile, next_page)) {
			
			this->page_id++;
			this->slot = 0;
			read_page(heapfile, this->page_id, this->page);
		} else {
			return false;
		}
	}
	
	return has_next;
}

Record RecordIterator::next() {
	Record r;
	if (heapfile->slot_size == COL_TUP_SIZE) {
		read_fixed_len_page_col(this->page, this->slot, &r);
	} else {
		read_fixed_len_page(this->page, this->slot, &r);
	}
	this->slot++;
	return r;
}


