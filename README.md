# shanghai
-- Big file sorter --

Goncharov Alexey for Altium.

Requirements:
	* CMake v2.8 or above;
	* C++11 compiler compatible with gcc 4.9 or above;
	* boost v 1.54 or above.

Building
	0. Go to project folder
	0. mkdir build
	0. cd build
	0. cmake ..
	0. make

This app was developed under Linux, it also can be built and run under MS Windows. 

generator
---------
	Generate random strings file.

	Usage: generator <file-name> <size>
	Example: generator data.txt 100G

sorter
------
	Sorts file of random strings.
	First it splits file to sorted chunks, which are stored in tmp files.
	Then it merges chunks to bigger chunks, until one file left.
	
	Usage: sorter <source-file> <result-file> <chunk size>
	Example: sorter data.txt result.txt 2G
	
	Chunk size should be about 1/4 of RAM size.

tests
-----
	Some unittests.
	Usage: tests
    
