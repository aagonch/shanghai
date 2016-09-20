
#include "common/Clock.h"

#include "SortingEntry.h"
#include "FileRegistry.h"
#include "InitialSorter.h"
#include "Merger.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

#include <boost/filesystem.hpp>

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
