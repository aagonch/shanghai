#pragma once

#include <boost/chrono.hpp>

class Clock
{
    boost::chrono::time_point<boost::chrono::system_clock> mStartTime;
public:
    void Start()
    {
        mStartTime = boost::chrono::system_clock::now();
    }

    double ElapsedTime()
    {
        auto diff = boost::chrono::system_clock::now() - mStartTime;
        return boost::chrono::duration_cast<boost::chrono::microseconds>(diff).count() * 1e-6;
    }
};
