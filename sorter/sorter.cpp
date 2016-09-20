#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <memory>
#include <algorithm>
#include <boost/optional.hpp>
#include "common/Clock.h"

#include "SortingEntry.h"
#include "FileReader.h"


#include <boost/filesystem.hpp>

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

    std::ofstream file(filename, std::ofstream::binary);

    if (!file)
        throw std::runtime_error("Cannot open output file.");

    for (const TEntry& entry : entries)
    {
        file << entry;
    }

    file.close();

    std::cout << "SaveFile complete, time:" << c.ElapsedTime() << "sec" << std::endl;
}

class FileRegistry
{
    size_t m_counter = 0;
    std::string m_initialFile;
    std::vector<std::string> m_files;
public:
    FileRegistry(const std::string& initialFile) : m_initialFile(initialFile) {}

    const std::string& GetInitialFile() const { return m_initialFile; }

    size_t Count() const { return m_files.size(); }

    std::string GetNext(const std::string& label = std::string())
    {
        std::string fname = m_initialFile + "." + label + (label.empty() ? "" : ".") + std::to_string(++m_counter);
        m_files.push_back(fname);
        return fname;
    }

    std::vector<std::string> PopFront(size_t count)
    {
        std::vector<std::string> result;
        if (m_files.size() <= count)
        {
            m_files.swap(result);
            return result;
        }
        result.assign(m_files.begin(), m_files.begin() + count);
        m_files.erase(m_files.begin(), m_files.begin() + count);
        return result;
    }
};

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
            const size_t aproxBytesPerLine = 20;
            entries.reserve(chunkSize / aproxBytesPerLine);
        }
    };

    size_t m_chunkSize;

public:
    InitialSorter(size_t chunkSize) : m_chunkSize(chunkSize)
    {

    }
    // returns file name list of sorted chunks
    void Process(FileRegistry& registry)
    {
        ChunkData<TEntry> chunk(m_chunkSize);
        ReadFile(chunk, registry);
    }

private:

    std::vector<std::string> ReadFile(ChunkData<SimpleEntry>& data, FileRegistry& registry)
    {
        std::ifstream file(registry.GetInitialFile().c_str());
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

        ProcessChunk(data, registry);

        std::cout << "Read complete, FileSize:" << fileSize*1e-6 << "M"
                  << ", EntryCount:" << data.entries.size()
                  << ", ReadTime:" << readTime << "sec"
                  << std::endl;
    }

    void ReadFile(ChunkData<FastEntry>& data, FileRegistry& registry)
    {
        FileReader reader(registry.GetInitialFile().c_str());

        Clock c;
        c.Start();
        size_t totalEntries = 0;
        while (reader.LoadNextChunk(data.buffer))
        {
            data.entries.clear();

            FileReader::Buffer line;
            while (reader.TryGetLine(&line))
            {
                ++totalEntries;
                data.entries.emplace_back(line.data, line.size);
//                std::cout << "LOADED [";
//                std::cout.write(line.data, line.size);
//                std::cout << "]" << std::endl;
            }
            double readTime = c.ElapsedTime();

            std::cout << "Chunk read complete, EntryCount:" << data.entries.size()
                      << ", ReadTime:" << readTime << "sec"
                      << std::endl;

            ProcessChunk(data, registry);
            c.Start();
        }

//        std::cout << "Read complete, FileSize:" << fileSize*1e-6 << "M"
//                  << ", EntryCount:" << data.entries.size()
//                  << ", ReadTime:" << readTime << "sec"
//                  << std::endl;
    }

    void ProcessChunk(ChunkData<FastEntry>& data, FileRegistry& registry)
    {
        Sort(data.entries);
        SaveFile(registry.GetNext().c_str(), data.entries);
    }
};

template<class TEntry>
class Merger
{
    struct Source
    {
        std::shared_ptr<FileReader> reader;
        std::shared_ptr<std::vector<char>> buffer;
        TEntry currentEntry;\

        void Next()
        {
            FileReader::Buffer line;
            if (!reader->TryGetLine(&line))
            {
                if (!reader->LoadNextChunk(buffer) || !reader->TryGetLine(&line))
                {
                    currentEntry = TEntry(); // invalidate entry
                    return;
                }
            }

            currentEntry = TEntry(line.data, line.size);
        }
    };

    std::vector<Source> m_sources;

public:
    Merger(size_t count, size_t readBufSize) : m_sources(count)
    {
        for (size_t n = 0; n < count; ++n)
        {
            m_sources[n].buffer = std::make_shared<std::vector<char>>(readBufSize);
        }
    }

    void Process(FileRegistry& registry)
    {
        int mergeIter = 0;
        for (; registry.Count() > 1; ++mergeIter)
        {
            std::vector<std::string> files = registry.PopFront(m_sources.size());
            assert(files.size() > 1);

            for (size_t n = 0; n < files.size(); ++n)
            {
                m_sources[n].reader = std::make_shared<FileReader>(files[n].c_str());
                m_sources[n].Next();
            }

            Clock c;
            c.Start();

            std::string outputFile = registry.GetNext("m");
            DoMergeIteration(outputFile, files.size());

            std::cout << "Merge #" << mergeIter << " complete for [";
            for (auto f : files) std::cout << f << "; ";
            std::cout << "] -> " << outputFile << ", Time:" << c.ElapsedTime() << std::endl;

            for (size_t n = 0; n < files.size(); ++n)
            {
                m_sources[n].reader.reset(); // close file
                boost::filesystem::remove(files[n]);
            }

            std::cout << "After deleting Time:" << c.ElapsedTime() << std::endl;
        }
    }

private:

    void DoMergeIteration(const std::string& outputFileName, size_t activeSourceCount)
    {
        std::ofstream file(outputFileName.c_str());

        size_t N = activeSourceCount;
        assert(N <= m_sources.size());

        for (;;)
        {
            size_t index = N; // index of source with min currentEntry (N means invalid)

            for (size_t n = 0; n < N; ++n)
            {
                ++yyy;
                if (!m_sources[n].currentEntry.IsValid())
                {
                    continue;
                }

                if (index == N || m_sources[n].currentEntry < m_sources[index].currentEntry)
                {
                    index = n;
                }
            }

            if (index < N)
            {
                // we found index of source with min entry
                // write entry to file and fetch next
                file << m_sources[index].currentEntry.get();
                m_sources[index].Next();
                continue;
            }

            break; // all sources has invalid items, stop
        }

        file.close();
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

        FileRegistry registry(argv[1]);

        //InitialSorter<SimpleEntry> sorter;

        size_t M = 1024*1024;
        InitialSorter<FastEntry> sorter(1024*M);
        sorter.Process(registry);

        Merger<FastEntry> merger(3, 100*M);
        merger.Process(registry);

        std::vector<std::string> result = registry.PopFront(100);
        assert(result.size() == 1);
        boost::filesystem::rename(result.at(0), argv[2]);

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
