#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>

#include "common/Clock.h"

#include "SortingEntry.h"
#include "FileReader.h"

template <class TEntry>
struct ChunkData
{
    std::vector<TEntry> entries;
    std::shared_ptr<std::vector<char>> buffer;
    size_t size;

    ChunkData(size_t chunkSize) : size(chunkSize)
    {
        if (TEntry::IsExternalBuffer)
        {
            buffer = std::make_shared<std::vector<char>>(chunkSize);

        }
        Reserve(chunkSize);
    }

    void Reserve(size_t chunkSize)
    {
        const size_t aproxBytesPerLine = 20;
        entries.reserve(chunkSize / aproxBytesPerLine);
    }
};

template <class TEntry>
void Sort(std::vector<TEntry>& entries)
{
    Clock c;
    c.Start();

    std::sort(entries.begin(), entries.end());

    std::cout << "Sort complete, time:" << c.ElapsedTime() << "sec" << std::endl;
}

template <class TEntry>
void SaveFile(const char* filename, const std::vector<TEntry>& entries)
{
    Clock c;
    c.Start();

    std::ofstream file(filename);

    if (!file)
        throw std::runtime_error("Cannot open output file.");

    for (const TEntry& entry : entries)
    {
        file << entry;
    }

    file.close();

    std::cout << "SaveFile complete, time:" << c.ElapsedTime() << "sec" << std::endl;
}

template <class TEntry>
class InitialSorter
{

public:
    InitialSorter()
    {

    }
    // returns file name list of sorted chunks
    std::vector<std::string> ProcessFile(const char* fileName, size_t chunkSize)
    {
        ChunkData<TEntry> chunk(chunkSize);
        return ReadFile(fileName, chunk);
    }

private:

    std::vector<std::string> ReadFile(const char* fileName, ChunkData<Entry>& data)
    {
        std::ifstream file(fileName);
        if (!file)
            throw std::logic_error("Cannot open file");

        file.seekg (0, file.end);
        size_t fileSize = file.tellg();
        file.seekg (0, file.beg);

        data.entries.clear();
        data.Reserve(fileSize);

        Clock c2;
        c2.Start();
        std::string line;
        while (std::getline(file, line))
        {
            data.entries.emplace_back(line.data(), line.size());
        }

        double readTime = c2.ElapsedTime();

        std::cout << "Read complete, FileSize:" << fileSize*1e-6 << "M"
                  << ", EntryCount:" << data.entries.size()
                  << ", ReadTime:" << readTime << "sec"
                  << std::endl;
    }

    std::vector<std::string> ReadFile(const char* fileName, ChunkData<FastEntry>& data)
    {
        data.entries.clear();

        std::vector<std::string> result;

        auto* reader = new FileReader(fileName);

        size_t fileSize = reader->GetFileSize();

        double readTime = 0;

        Clock c;
        c.Start();
        size_t totalEntries = 0;
        while (reader->LoadNextChunk(data.buffer))
        {
            FileReader::Buffer line;
            while (reader->TryGetLine(&line))
            {
                ++totalEntries;
                data.entries.emplace_back(line.data, line.size);
            }

            readTime += c.ElapsedTime();
            ProcessChunk(fileName, data, result);
            c.Start();
        }

        std::cout << "Read complete, FileSize:" << fileSize*1e-6 << "M"
                  << ", EntryCount:" << data.entries.size()
                  << ", ReadTime:" << readTime << "sec"
                  << std::endl;

        return result;
    }

    void ProcessChunk(const char* inputFile, ChunkData<FastEntry>& data, std::vector<std::string>& outputFiles)
    {
        std::string outputFile = inputFile;
        outputFile += "." + std::to_string(outputFiles.size());
        outputFiles.push_back(outputFile);

        Sort(data.entries);

        SaveFile(outputFile.c_str(), data.entries);
    }
};

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: sorter <input-file> <output-file>" << std::endl;
        return 1;
    }

    try
    {

        Clock c;
        c.Start();

        //InitialSorter<Entry> sorter;
        InitialSorter<FastEntry> sorter;
        sorter.ProcessFile(argv[1], 1100*1024*1024);

        std::cout << "Success, totalTime:" << c.ElapsedTime() << "sec"
                  << ", xxx:" << xxx << ""
                  << ", yyy:" << yyy << ""
                  << std::endl;
    }
    catch(std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;



}
