'''
Python script for Part 1.1 investigating the effect of block size of file writes on performance
'''
from typing import *
from subprocess import call

KILOBYTE = 1024
MEGABYTE = KILOBYTE * 1024
TOTAL_BYTES = 3 * MEGABYTE
BLOCK_SIZES = [100, 512, KILOBYTE, 32*KILOBYTE, 128*KILOBYTE, 256*KILOBYTE, 512*KILOBYTE, MEGABYTE, 2*MEGABYTE, 3*MEGABYTE]
FILENAME_PREFIX = "rf_{}bs"
#FILENAME_PREFIX = "/Volumes/KINGSTON/rf_{}bs"

def execute_command(filename:str, total_bytes:int, block_size:int) -> int:
	call(["./create_random_file", filename, str(total_bytes), str(block_size)])

def generate_filename(size:int) -> str:
	return FILENAME_PREFIX.format(size)	

for size in BLOCK_SIZES:
	filename = generate_filename(size)
	execute_command(filename, TOTAL_BYTES, size)
