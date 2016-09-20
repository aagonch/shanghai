#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cassert>
#include <algorithm>

#include "xxhash/xxhash.h"

#include "Utils.h"

// some platforms may define macros 'min', undef it!
#ifdef min
#undef min
#endif


// Entry we are going to sort.
class SimpleEntry
{
    int m_number;
    std::string m_string;
public:
    static constexpr bool IsExternalBuffer = false;
    SimpleEntry() : m_number(-1) {}
    SimpleEntry(int number, const std::string& string) : m_number(number), m_string(string) {}
    SimpleEntry(const char* buff, size_t size)
    {
        const char* dot = strchr(buff, '.');
        if (dot == nullptr || dot[1] != ' ')
            throw std::logic_error("Invalid line");

        m_number = atoi(buff);
        m_string.assign(dot + 2, buff + size); // begin=(dot + 2) because of dot and space befoe string begins.
    }

    bool IsValid() const { return m_number >= 0; }

    bool operator<(const SimpleEntry& other) const
    {
        int cmp = strcmp(m_string.c_str(), other.m_string.c_str());

        if (cmp < 0) return true;
        return cmp == 0 && m_number < other.m_number;
    }

    friend std::ostream& operator<<(std::ostream& os, const SimpleEntry& entry)
    {
        return os << entry.m_number << ". " << entry.m_string;
    }
};


size_t xxx = 0;
size_t yyy = 0;

template <size_t PrefixLen> struct PrefixSelector;
template <> struct PrefixSelector<1> { typedef std::tuple<uint64_t> Type; };
template <> struct PrefixSelector<2> { typedef std::tuple<uint64_t, uint64_t> Type; };
template <> struct PrefixSelector<3> { typedef std::tuple<uint64_t, uint64_t, uint64_t> Type; };
template <> struct PrefixSelector<4> { typedef std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> Type; };

// Entry we are going to sort.
template<size_t PrefixLen, bool UseHash_>
class TFastEntry
{
    int m_number = -1;
    typename PrefixSelector<PrefixLen>::Type m_prefix;
    XXH64_hash_t m_hash = 0;
    const char* m_linePtr = nullptr;
    const char* m_strPtr = nullptr;
    size_t m_lineSize = 0;

public:

    static constexpr bool IsExternalBuffer = true;
    static constexpr bool UseHash = UseHash_;

    TFastEntry() {}

    TFastEntry(const char* line, size_t size) : m_linePtr(line), m_lineSize(size)
    {
        m_number = atoi(line);
        const char* dotPos = reinterpret_cast<const char*>(memchr(line, '.', size));
        if (dotPos == nullptr)
            throw std::logic_error("Invalid line [" + std::string(line, line + std::min(1000LU, size)) + "]");

        const char* str = dotPos + 1;
        const char* end = line + size;

        size_t strSize = end - str;

        //m_prefix = GetPrefix128(str, strSize);
        GetPrefixTuple(str, strSize, &m_prefix);
        if (UseHash)
        {
            const size_t seed = 43;
            m_hash = XXH64(str, strSize, seed);
        }

        m_strPtr = str;
    }

    bool IsValid() const { return m_linePtr != nullptr; }

    XXH64_hash_t GetHash() const { return m_hash; }

    bool operator<(const TFastEntry& other) const
    {
        if (m_prefix < other.m_prefix) return true;
        if (m_prefix > other.m_prefix) return false;

        // first N bytes of strings are equal (N = sizeof(m_prefix)).

        if (!UseHash || m_hash != other.m_hash)
        {
            ++xxx;
            const char* end = m_linePtr + m_lineSize;
            const char* end1 = other.m_linePtr + other.m_lineSize;
            size_t size = end - m_strPtr;
            size_t size1 = end1 - other.m_strPtr;

            constexpr size_t N = sizeof(m_prefix);

            bool isShortStringEquality = size <= N && size == size1;

            if (!isShortStringEquality)
            {
                // it not short strings with equal prefixes, compare futher

                // start cmp after N bytes.
                int cmp = memcmp(m_strPtr + N, other.m_strPtr + N, std::min(size, size1) - N);

                if (cmp < 0) return true;
                if (cmp > 0) return false;

                if (size < size1) return true;
                if (size > size1) return false;
            }
        }

        // strings are equal, compare numbers
        return m_number < other.m_number;
    }

    friend std::ostream& operator<<(std::ostream& os, const TFastEntry& entry)
    {
        static const std::string eol = GetPlatformEol();
        os.write(entry.m_linePtr, entry.m_lineSize);
        os.write(&eol[0], eol.size()); // Do not use std::endl, because it flushs the stream.
        return os;
    }
};

typedef TFastEntry<2, false> FastEntry;
