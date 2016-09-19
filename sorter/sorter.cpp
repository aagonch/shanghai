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

    ChunkData(size_t chunkSize)
    {
        if (chunkSize > 0)
        {
            buffer = std::make_shared<std::vector<char>>(chunkSize);
            Reserve(chunkSize);
        }
    }

    void Reserve(size_t chunkSize)
    {
        const size_t aproxBytesPerLine = 20;
        entries.reserve(chunkSize / aproxBytesPerLine);
    }
};

template <class TEntry>
class InitialSorter
{
    ChunkData<TEntry> m_chunk;
private:

    InitialSorter() : m_chunk(1024*1024*1024) {}

    void ReadFile(const char* fileName, std::vector<Entry>& entries)
    {
        std::ifstream file(fileName);
        if (!file)
            throw std::logic_error("Cannot open file");


        file.seekg (0, file.end);
        size_t fileSize = file.tellg();
        file.seekg (0, file.beg);

        entries.clear();

        Clock c2;
        c2.Start();
        std::string line;
        while (std::getline(file, line))
        {
            entries.emplace_back(line.data(), line.size());
        }

        double readTime = c2.ElapsedTime();

        std::cout << "Read complete, FileSize:" << fileSize*1e-6 << "M"
                  << ", EntryCount:" << entries.size()
                  << ", ReadTime:" << readTime << "sec"
                  << std::endl;
    }

    void ReadFile(const char* fileName, std::vector<FastEntry>& entries)
    {
        const size_t aproxBytesPerLine = 20;

        entries.clear();

        Clock c2;
        c2.Start();
        auto* reader = new FileReader(fileName);

        size_t fileSize = reader->GetFileSize();

        size_t reserveCount = fileSize / aproxBytesPerLine;
        entries.reserve(reserveCount);

        auto chunk = std::make_shared<std::vector<char>>(1500*1024*1024);
        while (reader->LoadNextChunk(chunk))
        {
            FileReader::Buffer line;
            while (reader->TryGetLine(&line))
            {
               entries.emplace_back(line.data, line.size);
            }
            break;
        }

        double readTime = c2.ElapsedTime();

        std::cout << "Read complete, FileSize:" << fileSize*1e-6 << "M"
                  << ", EntryCount:" << entries.size()
                  << ", ReadTime:" << readTime << "sec"
                  << std::endl;
    }

    void Sort(std::vector<TEntry>& entries)
    {
        Clock c;
        c.Start();

        std::sort(entries.begin(), entries.end());

        std::cout << "Sort complete, time:" << c.ElapsedTime() << "sec" << std::endl;
    }

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
        std::vector<Entry> entries;
        //std::vector<FastEntry> entries;
        Clock c;
        c.Start();

        ReadFile(argv[1], entries);

        Sort(entries);

        SaveFile(argv[2], entries);

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
