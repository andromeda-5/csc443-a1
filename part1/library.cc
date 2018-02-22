#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>

#include "library.h"

void random_array(char *array, long bytes){
	int i;
	char c;
	for (i = 0; i < bytes; i++) {
		c = 'A' + rand() % 26;
		array[i] = c;
	}
}

