#pragma once

#include "FileRegistry.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "SortingEntry.h"

#include "common/Clock.h"

#include <cassert>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

#include <boost/filesystem.hpp>


// reads
template<class TEntry>
class Merger
{
    struct Source
    {
        std::shared_ptr<FileReader> reader;
        std::shared_ptr<std::vector<char>> buffer;
        TEntry currentEntry;

        // Gets next entry from source
        void Next(double& pureReadTime)
        {
            FileReader::Buffer line;
            if (!reader->TryGetLine(&line))
            {
                Clock c;
                c.Start();
                if (!reader->LoadNextChunk(buffer) || !reader->TryGetLine(&line))
                {
                    currentEntry = TEntry(); // invalidate entry
                    return;
                }
                pureReadTime += c.ElapsedTime();
            }

            currentEntry = TEntry(line.data, line.size);
        }
    };

    std::vector<Source> m_sources;
    double m_pureReadTime;

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
                m_sources[n].Next(m_pureReadTime);
            }

            Clock c;
            c.Start();

            std::string outputFile = registry.GetNext("m");
            DoMergeIteration(outputFile, files.size());

            std::cout << "Merge #" << mergeIter << " complete for [";
            for (auto f : files) std::cout << f << "; ";
            std::cout << "] -> " << outputFile;
            std::cout << ", Time:" << c.ElapsedTime() << "s, PureReadTime:" << m_pureReadTime << "s" <<std::endl;

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
                m_sources[index].currentEntry.ToStream(file);
                m_sources[index].Next(m_pureReadTime);
                continue;
            }

            break; // all sources has invalid items, stop
        }

        file.close();
    }
};
