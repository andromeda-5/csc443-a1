#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/timeb.h>
#include <math.h>

#include "library.h"

int main(int argc, char **argv){
	// argument processing
	if (argc != 4) {
		printf("Usage: create_random_file <filename> <total bytes> <block size>\n");
		return 1;
	}
	char *filename_tmp = argv[1];
	int total_bytes = strtol(argv[2], NULL, 10);
	int block_size = strtol(argv[3], NULL, 10);

	char *filename = (char *)malloc(sizeof(char) * (strlen(filename_tmp) + 1));
	strncpy(filename, filename_tmp, sizeof(char) * strlen(filename_tmp));
	filename[strlen(filename_tmp)] = '\0';

	struct timeb start, end;
	char *buffer = (char *) malloc(sizeof(char) * block_size);
	FILE *fp = fopen(filename, "w"); 
	if (!fp) {
		perror("fopen failed");
		return 1;
	}

	long total_time = 0;
	for (int i=0; i < total_bytes; i+= block_size) {
		//write random bytes to buffer
		random_array(buffer, block_size);
		//time the write operation
		ftime(&start);
		fwrite(buffer, 1, block_size, fp);
		fflush(fp);
		ftime(&end);
		total_time += (end.time * 1000 + end.millitm) - (start.time * 1000 + start.millitm);
	}

	fclose(fp); 
	printf("It took: %lu ms to write %d total bytes in %d block_size to the file '%s'\n", total_time, total_bytes, block_size, filename);
	free(filename);
	free(buffer);
	return 0;
} 