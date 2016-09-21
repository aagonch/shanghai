
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
    if (argc != 4)
    {
        std::cerr << "Usage: sorter <input-file> <output-file> <chunk-size>" << std::endl;
        return 1;
    }

    try
    {
        Clock c;
        c.Start();

        FileRegistry registry(argv[1]);

        InitialSorter<FastEntry> sorter(GetSize(argv[3]));
        //InitialSorter<SimpleEntry> sorter;
        sorter.Process(registry);

        std::cout << "Merging, totalTime:" << c.ElapsedTime() << "sec" << std::endl;

        Merger<FastEntry> merger(8, GetSize("32M"));
        merger.Process(registry);

        std::cout << "Renaming, totalTime:" << c.ElapsedTime() << "sec" << std::endl;

        std::vector<std::string> result = registry.PopFront(100);
        assert(result.size() == 1);
        boost::filesystem::rename(result.at(0), argv[2]);

        std::cout << "Success, totalTime:" << c.ElapsedTime() << "sec"
                  << ", totalCmpCount:" << totalCmpCount << ""
                  << ", memCmpCount:" << memCmpCount << ""
                  << std::endl;
    }
    catch(std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
