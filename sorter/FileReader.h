#pragma once

#include <cstdio>
#include <memory>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cassert>
#include "boost/noncopyable.hpp"

#include "Utils.h"

struct FileReader : boost::noncopyable
{
    struct Buffer
    {
        const char* data;
        size_t size;
    };

    FileReader(const char* fileName, const char* eol = nullptr)
    {
        m_file = fopen(fileName , "rb" );
        if (m_file == nullptr)
            throw std::runtime_error(std::string("Cannot open file ") + fileName);

        // obtain file size:
        fseek (m_file , 0 , SEEK_END);
        m_fileSize = ftell (m_file);
        rewind (m_file);

        if (eol && *eol)
        {
            m_eol = eol;
        }
        else
        {
            m_eol = GetPlatformEol();
        }
        assert(!m_eol.empty());
        m_actualEol = m_eol[0];
    }

    ~FileReader()
    {
        fclose(m_file);
    }

    // reads chunk from file to buffer
    bool LoadNextChunk(const std::shared_ptr<std::vector<char>>& newBuffer)
    {
        if (m_remained > 0)
        {
            if (newBuffer->size() < m_remained)
                throw std::logic_error("Invalid chunk buffer size");

            memmove(&newBuffer->front(), m_nextLinePos, m_remained);
        }

        m_buffer = newBuffer;

        m_nextLinePos = &m_buffer->front();
        size_t bytesToRead = m_buffer->size() - m_remained;

        if (bytesToRead > 0)
        {
            size_t bytesRead = fread(&m_buffer->at(m_remained), 1u, bytesToRead, m_file);
            assert(bytesRead <= bytesToRead);
            m_remained += bytesRead;
            return bytesRead > 0;
        }

        return true;
    }

    bool TryGetLine(Buffer* lineBuffer)
    {
        if (m_nextLinePos == nullptr) return false;

        const char* nextEolPos = reinterpret_cast<const char*>(memchr(m_nextLinePos, m_actualEol, m_remained));

        if (nextEolPos == nullptr)
        {
            if (m_remained > 0 && feof(m_file))
            {
                // we reach EOF instead of EOL.
                // Return the last line, next TryGetLine() calls will fail.
                lineBuffer->data = m_nextLinePos;
                lineBuffer->size = m_remained;

                m_remained = 0;

                return true;
            }

            // The first part of line came to this chunk,
            // The other part come to the next chunk.

            return false;
        }

        size_t lineSize = PtrDiff(m_nextLinePos, nextEolPos);
        size_t consumedBytes = lineSize + m_eol.size();

        lineBuffer->data = m_nextLinePos;
        lineBuffer->size = lineSize;

        assert(m_remained >= consumedBytes);
        m_nextLinePos += consumedBytes;
        m_remained -= consumedBytes;

        return true;
    }

    size_t GetFileSize() const { return m_fileSize; }

private:

    static size_t PtrDiff(const char* p0, const char* p1)
    {
        auto diff = p1 - p0;
        assert(diff >= 0);
        return static_cast<size_t>(diff);
    }

    FILE* m_file;
    size_t m_fileSize;
    std::shared_ptr<std::vector<char>> m_buffer;
    const char* m_nextLinePos = nullptr;
    size_t m_remained = 0;
    std::string m_eol;
    char m_actualEol;
};
