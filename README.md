# shanghai
-- Big file sorter --

Goncharov Alexey for Altium.

Ported to Windows
Requirements:
	* Visual Studio 2010 or higher,
    * boost 1.58 or higher

    
To build define environment vars
	0. set BOOST_INCLUDE_DIR=<path to boost headers>
	0. set BOOST_LIBRARYDIR=<path to boost libs x86>
	0. set BOOST_LIBRARYDIR64=<path to boost libs x64>
    --- OR ---
    Setup correct INCLUDE & LINK directories in vcprojs.
    
    
-- Changes -- 
    0. Added vcprojs
    0. Everywhere removed C++11 features to make compatible with MSVC 2010 installed on my collegue's laptop.
    0. Fixed rand() in generator (RAND_MAX=MAX_INT on Linux vs 0x7fff on Win).
    0. Fixed line endings for win

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
	
	Chunk size should be:
        * x86 version: 750M 
        * x64 version: about 1/4 of RAM size.
    x64 version is prefered, it works 10%-15% faster than x86 even with same chunk size.

tests
-----
	Some unittests.
	Usage: tests
    
