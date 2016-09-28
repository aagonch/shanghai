#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cctype>
#include <vector>

#include <boost/circular_buffer.hpp>

#include "common/Clock.h"
#include "common/Utils.h"

extern const char* words[];

const int MaxWords = 10;
const int MaxNum = 1000000;
const size_t RepeatBufferSize = 100;

size_t GetWordsCount();
size_t GetFileSize(const std::string& param);
std::string MakeNewString();
void Generate(const char* fileName, size_t requestedSize);
void PrintStat();
int MyRand();

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: generator <output-file> <file-size>" << std::endl;
        PrintStat();
        return 1;
    }

    srand(static_cast<unsigned int>(time(0)));

    try
    {
        const size_t requestedSize = GetSize(argv[2]);

        Clock clock;
        clock.Start();

        Generate(argv[1], requestedSize);

        std::cout << "Success! Elapsed time: " << clock.ElapsedTime() << " sec."<< std::endl;
    }
    catch(std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void Generate(const char* fileName, size_t requestedSize)
{
    std::ofstream file(fileName);
    if (!file)
        throw std::logic_error("Cannot open file");

    // Task conditions requires that some strings are met more than 1 time in the file
    // Store recent strings and insert them second time sometimes
    boost::circular_buffer<std::string> recentStrings(RepeatBufferSize);

    while (file.tellp() < requestedSize)
    {
        int num = MyRand() % MaxNum;

        file << num << ".";

        const int magicNumber = 42;
        bool usePrevString = !recentStrings.empty() && (MyRand() % magicNumber == 0);
        if (usePrevString)
        {
            // get random string from recentStrings
            size_t index = static_cast<size_t>(MyRand() % recentStrings.size());
            file << recentStrings[index];
        }
        else
        {
            // generate a new string
            recentStrings.push_back(MakeNewString());
            file << recentStrings.back();
        }

        file << std::endl;
    }

    file.close();
}

size_t GetWordsCount()
{
    size_t n = 0;
    while(words[n]) ++n;
    return n;
}

// return new string (always started with space).
std::string MakeNewString()
{
    static const size_t totalWords = GetWordsCount();

    std::string result;
    result.reserve(50);
    int numWords = rand() % MaxWords + 1;
    for (int n = 0; n < numWords; ++n)
    {
        size_t wordIndex = MyRand() % totalWords;
        result += " ";
        result += words[wordIndex];
    }
    return result;
}

void PrintStat()
{
    size_t N = 0;
    size_t totalLen = 0;
    std::vector<size_t> lengthDistr;

    while (const char* word = words[N])
    {
        size_t len = strlen(word);
        totalLen += len;

        if (lengthDistr.size() < len + 1)
            lengthDistr.resize(len + 1);

        lengthDistr[len] += 1;
        ++N;
    }

    std::cout << "Words in dictionary: " << N << std::endl;
    std::cout << "Average word length: " << totalLen * 1.0  / N << std::endl;

    std::cout << "Word length disrtibution: " << std::endl;

    for (size_t n = 0; n < lengthDistr.size(); ++n)
    {
        std::cout << n << ":" << lengthDistr[n] << "(" << lengthDistr[n] * 100.0 / N << "%)" << std::endl;
    }
}

// crossplatform rand()
int MyRand()
{
	if (RAND_MAX < MaxNum)
	{
		int N = 32*1024;
		int a = rand() % N, b = rand() % N;
		return a*N + b; 
	}
	
	return rand();
}
