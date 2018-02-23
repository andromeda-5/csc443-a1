<snippet>


# 1:CSC443 A1 Part1

## Description

In this part of the assignment we had to investigate the data access characteristics of secondary storage devices. 

## Functions 

1. Allocates a fixed amount of memory char buffer[block size] and repeatedly generates random content into buffer, which is then written to disk.

`create_random_file <filename> <total bytes> <block size>`

## Scripts

To run automated scripts testing the performance of these functions type the following in the terminal: 

1. To test the write data rate for 10 different block sizes in the range of 100B to 3MB:

`python3 write_expr.py`

2. To test the read data rate for 10 different block sizes in the range of 100B to 3MB:

`python3 read_expr.py`


</snippet>