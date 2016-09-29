#pragma once

#include "common/Utils.h"

#include <iostream>
#include <string>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <tuple>
#include <stdint.h>

// Entry we are going to sort, primitive implementation (for comparing)

size_t totalCmpCount = 0;
size_t memCmpCount = 0;

// Entry we are going to sort.
// features:
// * no memory copy, it stores ptr to buffer
// * memcpy() instead of lexical_cmp
// sizeof == 16 byte to allow bigger chunk size
// Time of std::sort of 1G data is ~17sec
class SmallEntry
{
    const char* m_linePtr;
    uint64_t m_packedData;


protected:

    const char* GetLinePtr() const { return m_linePtr; }
    size_t GetNumber() const { return m_packedData >> 32; }
    size_t GetLineSize() const { return (m_packedData >> 16) & 0xffff; }
    size_t GetStringOffset() const { return m_packedData & 0xffff; }

    const char* GetStringPtr() const { return m_linePtr + GetStringOffset(); }
    size_t GetStringLen() const
    {
        assert(GetLineSize() >= GetStringOffset());
        return GetLineSize() - GetStringOffset();
    }
public:

    static const bool IsExternalBuffer = true;
    static const bool UseHash = false;

    SmallEntry() : m_linePtr(0), m_packedData(0) {}

    SmallEntry(const char* line, size_t size) : m_linePtr(line)
    {
        assert(size <= 0xffff);

        const char* dotPos = reinterpret_cast<const char*>(memchr(line, '.', size));
        if (dotPos == nullptr)
            throw std::logic_error("Invalid line [" + std::string(line, line + size) + "]");

        size_t offset = dotPos - line + 1;
        m_packedData = fast_atoi(line);
        m_packedData = m_packedData << 32;
        m_packedData = m_packedData | ((size & 0xffff) << 16);
        m_packedData = m_packedData | ((offset & 0xffff) << 0);
    }

    bool IsValid() const { return m_linePtr != nullptr; }

    size_t GetHash() const { return 0; }

    bool operator<(const SmallEntry& other) const
    {
        assert(IsValid());
        assert(other.IsValid());

        size_t size = GetStringLen();
        size_t size1 = other.GetStringLen();

        int cmp = memcmp(GetStringPtr(), other.GetStringPtr(), std::min(size, size1));

        if (cmp < 0) return true;
        if (cmp > 0) return false;

        if (size < size1) return true;
        if (size > size1) return false;

        return GetNumber() < other.GetNumber();
    }

    template <class TStream>
    void ToStream(TStream& stream) const
    {
        static const std::string eol = GetPlatformEol();
        stream.write(GetLinePtr(), GetLineSize());
        stream.write(&eol[0], eol.size()); // Do not use std::endl, because it flushs the stream.
    }
};

static_assert(sizeof(SmallEntry) == 16, "check SmallEntry");

// Extends SmallEntry
// * first N bytes comparing as int64, not as array of char (N=2*sizeof(uint64))
// Larger sizeof, but faster compare.
// Time of std::sort of 1G data is ~3.5sec
class FastEntry : public SmallEntry
{
    uint64_t m_prefix[2];
public:

    FastEntry() {}
    FastEntry(const char* line, size_t size) : SmallEntry(line, size)
    {
        GetPrefixTuple(GetStringPtr(), GetStringLen(), &m_prefix[0], &m_prefix[1]);
    }

    bool operator<(const FastEntry& other) const
    {
        ++totalCmpCount;
        if (m_prefix[0] < other.m_prefix[0]) return true;
        if (m_prefix[0] > other.m_prefix[0]) return false;

		if (m_prefix[1] < other.m_prefix[1]) return true;
        if (m_prefix[1] > other.m_prefix[1]) return false;
        // first N bytes of strings are equal (N = sizeof(m_prefix)).

        size_t size = GetStringLen();
        size_t size1 = other.GetStringLen();

        const size_t N = sizeof(uint64_t)*2;

        if (size > N && size1 > N)
        {
            int cmp = memcmp(GetStringPtr() + N, other.GetStringPtr() + N, std::min(size, size1) - N);

            if (cmp < 0) return true;
            if (cmp > 0) return false;

            if (size < size1) return true;
            if (size > size1) return false;
        }

        // strings are equal, compare numbers
        return GetNumber() < other.GetNumber();
    }
};

static_assert(sizeof(FastEntry) <= 32U, "check FastEntry");
