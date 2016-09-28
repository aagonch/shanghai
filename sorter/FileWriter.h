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

    for (size_t n = 0; n < entries.size(); ++n)
    {
        entries[n].ToStream(file);
    }

    file.close();

    std::cout << "SaveFile(" << filename << ") complete, time:" << c.ElapsedTime() << "sec" << std::endl;
}
