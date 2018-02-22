#include <stdlib.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <assert.h>
#include <string.h>

#include "library.h"

/**
 * file_ptr: the file pointer, ready to be read from.
 * hist: an array to hold 26 long integers. hist[0] is the
 *		 number of 'A' and hist[1] is the number of 'B', etc.
 * block_size the buffer size to be used.
 * milliseconds: time it took to complete the file scan
 * total_bytes_read: the amount of data in bytes read
 *
 * returns: -1 if there is an error.
 */
int get_historgram(FILE *file_ptr, long hist[], int block_size, long *milliseconds, long *total_bytes_read) {
	
	char buffer[block_size];
	int ret_val;
	long index;
	struct timeb start, end;

	for (long j = 0; j < *total_bytes_read; j += block_size) {

		ftime(&start);
		ret_val = fread(buffer, sizeof(char), block_size, file_ptr);
		ftime(&end);

		*milliseconds += (end.time * 1000 + end.millitm) - (start.time * 1000 + start.millitm);
		if (ret_val < 0) {
			perror("fread error\n");
			return 1;
		}

		for (int i=0; i<block_size; i++) {
			index = buffer[i] - 'A';
			hist[index] += 1;
		}
	}
	
	return 0;
}

int main(int argc, char **argv){

	if (argc != 3) {
		printf("Usage: get_historgram <filename> <blocksize> \n");
		return 1;
	}
	
	char *filename_tmp = argv[1];
	int block_size = (int) strtol(argv[2], NULL, 10);
	char *filename = (char *)malloc(sizeof(char) * (strlen(filename_tmp) + 1));
	strncpy(filename, filename_tmp, sizeof(char) * strlen(filename_tmp));
	filename[strlen(filename_tmp)] = '\0';

	long hist[26];
	long milliseconds;
	long filelen;

	memset(hist, 0, sizeof(hist));
	FILE *file_ptr = fopen(filename , "r");
	if (!file_ptr) {
		perror("fopen error\n");
		return 1;
	}

	fseek(file_ptr, 0, SEEK_END);
	filelen = ftell(file_ptr);
	if (filelen == -1){
		perror("ftell error\n");
		return 1;
	}
	//wind back
	rewind(file_ptr);

	/**
	 * Compute the histogram using 2K buffers
	 */
	int ret = get_historgram(file_ptr, hist, block_size, &milliseconds, &filelen);
	assert(ret == 0);

	printf("Computed the histogram in %lu ms.\n", milliseconds);
	for (int i = 0; i < 26; i++) {
		printf("%c : %lu\n", 'A' + i, hist[i]);
	}

	printf("BLOCK_SIZE %d bytes\n", block_size);
	printf("TOTAL_BYTES %lu bytes\n", filelen);
	printf("TIME %lu milliseconds\n", milliseconds);

	printf("Data rate: %f Bps\n", (double) filelen/milliseconds * 1000);

	fclose(file_ptr);
	return 0;
}