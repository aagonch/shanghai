#pragma once

#include "FileRegistry.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "SortingEntry.h"

#include "common/Clock.h"

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <algorithm>

template <class TEntry>
void Sort(std::vector<TEntry>& entries)
{
    Clock c;
    c.Start();

    std::sort(entries.begin(), entries.end());

    std::cout << "Sort complete, time:" << c.ElapsedTime() << "sec" << std::endl;
}

// Reads source file and splits it to sorted chunks.
template <class TEntry>
class InitialSorter
{
    template <class TEntry_>
    struct ChunkData
    {
        std::vector<TEntry_> entries;
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
            const size_t aproxBytesPerLine = 32;
            entries.reserve(chunkSize / aproxBytesPerLine);
        }
    };

    size_t m_chunkSize;

public:
    InitialSorter(size_t chunkSize) : m_chunkSize(chunkSize)
    {
    }

    void Process(FileRegistry& registry)
    {
        ChunkData<TEntry> chunk(m_chunkSize);
        ReadFile(chunk, registry);
    }

private:

    // the main version of ReadFile
    void ReadFile(ChunkData<TEntry>& data, FileRegistry& registry)
    {
        FileReader reader(registry.GetInitialFile().c_str());

        Clock c;
        c.Start();
        size_t totalEntries = 0;
        int n = 0;
        while (reader.LoadNextChunk(data.buffer))
        {
            data.entries.clear();

            FileReader::Buffer line;
            while (reader.TryGetLine(&line))
            {
                ++totalEntries;
                data.entries.push_back(TEntry(line.data, line.size));
            }
            double readTime = c.ElapsedTime();

            std::cout << "Chunk read complete, EntryCount:" << data.entries.size()
                      << ", ReadTime:" << readTime << "sec"
                      << std::endl;

            ProcessChunk(data, registry);
            c.Start();
        }
    }

    void ProcessChunk(ChunkData<TEntry>& data, FileRegistry& registry)
    {
        Sort(data.entries);
        SaveFile(registry.GetNext().c_str(), data.entries);
    }
};
