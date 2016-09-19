#pragma once

#include <chrono>

class Clock
{
    std::chrono::time_point<std::chrono::system_clock> mStartTime;
public:
    void Start()
    {
        mStartTime = std::chrono::system_clock::now();
    }

    double ElapsedTime()
    {
        auto diff = std::chrono::system_clock::now() - mStartTime;
        return std::chrono::duration_cast<std::chrono::microseconds>(diff).count() * 1e-6;
    }
};
