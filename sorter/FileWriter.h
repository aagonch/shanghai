#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "common/Clock.h"

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
