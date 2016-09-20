#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <tuple>


inline uint64_t ReverseByteOrder(uint64_t value)
{
    union
    {
        uint64_t value;
        char buffer[8ul];
    } x;
    x.value = value;

    // x86 is little endian, we must reverse byte order.
//    std::swap(x.buffer[0u], x.buffer[7u]);
//    std::swap(x.buffer[1u], x.buffer[6u]);
//    std::swap(x.buffer[2u], x.buffer[5u]);
//    std::swap(x.buffer[3u], x.buffer[4u]);
//    return x.result;

    return  (uint64_t(x.buffer[0u]) << 56) |
            (uint64_t(x.buffer[1u]) << 48) |
            (uint64_t(x.buffer[2u]) << 40) |
            (uint64_t(x.buffer[3u]) << 32) |
            (uint64_t(x.buffer[4u]) << 24) |
            (uint64_t(x.buffer[5u]) << 16) |
            (uint64_t(x.buffer[6u]) <<  8) |
            (x.buffer[7u]);
}

inline void GetPrefixTuple(const char* str, size_t size, std::tuple<uint64_t>* result)
{
    uint64_t data[1] = {0};
    memcpy(&data, str, std::min(sizeof(data), size));

    std::get<0>(*result) = ReverseByteOrder(data[0]);
}


// 64bit is not enough, we would like to use uint128_t.
// It is available in gcc, for portability reason use tuples, they
inline void GetPrefixTuple(const char* str, size_t size, std::tuple<uint64_t, uint64_t>* result)
{
    uint64_t data[2] = {0};
    memcpy(&data, str, std::min(sizeof(data), size));

    std::get<0>(*result) = ReverseByteOrder(data[0]);
    std::get<1>(*result) = ReverseByteOrder(data[1]);
}

inline void GetPrefixTuple(const char* str, size_t size, std::tuple<uint64_t, uint64_t, uint64_t>* result)
{
    uint64_t data[3] = {0};
    memcpy(&data, str, std::min(sizeof(data), size));

    std::get<0>(*result) = ReverseByteOrder(data[0]);
    std::get<1>(*result) = ReverseByteOrder(data[1]);
    std::get<2>(*result) = ReverseByteOrder(data[1]);
}

inline void GetPrefixTuple(const char* str, size_t size, std::tuple<uint64_t, uint64_t, uint64_t, uint64_t>* result)
{
    uint64_t data[4] = {0};
    memcpy(&data, str, std::min(sizeof(data), size));

    std::get<0>(*result) = ReverseByteOrder(data[0]);
    std::get<1>(*result) = ReverseByteOrder(data[1]);
    std::get<2>(*result) = ReverseByteOrder(data[2]);
    std::get<3>(*result) = ReverseByteOrder(data[3]);
}

// instead of comparing strings, we compare 64bit integers
inline uint64_t GetPrefix(const char* str, size_t size)
{
    uint64_t data = 0;
    memcpy(&data, str, std::min(sizeof(data), size));
    return ReverseByteOrder(data);
}

inline std::string GetPlatformEol()
{
    std::stringstream ss;
    ss << std::endl;
    return ss.str();
}
