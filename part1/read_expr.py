'''
Python script for Part1.2 investigating the effect of block size of file reads on performance
'''
from typing import *
from subprocess import call

KILOBYTE = 1024
MEGABYTE = KILOBYTE * 1024
TOTAL_BYTES = 3 * MEGABYTE
BLOCK_SIZES = [100, 512, KILOBYTE, 32*KILOBYTE, 128*KILOBYTE, 256*KILOBYTE, 512*KILOBYTE, MEGABYTE, 2*MEGABYTE, 3*MEGABYTE]

def execute_command(filename:str, block_size:int) -> int:
	call(["./get_histogram", filename, str(block_size)])

for size in BLOCK_SIZES:
	filename = "/Volumes/KINGSTON/mybigfile.txt"
	execute_command(filename, size)