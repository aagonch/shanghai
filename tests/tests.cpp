#define BOOST_TEST_MODULE MyTest

#include <boost/test/unit_test.hpp>
#include <fstream>

#include "sorter/FileReader.h"
#include "sorter/SortingEntry.h"

inline std::string ToStr(const FileReader::Buffer& b)
{
    return std::string(b.data, b.data + b.size);
}

static const char* filename = "test.txt";

static void Expect_AAA_BBB_DDD(FileReader& reader, const std::shared_ptr<std::vector<char>>& chunk)
{
    FileReader::Buffer b;
    BOOST_CHECK(reader.LoadNextChunk(chunk));

    BOOST_CHECK(reader.TryGetLine(&b));
    BOOST_CHECK_EQUAL("AAA", ToStr(b));

    BOOST_CHECK(reader.TryGetLine(&b));
    BOOST_CHECK_EQUAL("BBB", ToStr(b));

    BOOST_CHECK(reader.TryGetLine(&b));
    BOOST_CHECK_EQUAL("DDD", ToStr(b));

    BOOST_CHECK(!reader.TryGetLine(&b));
    BOOST_CHECK(!reader.LoadNextChunk(chunk));
}

BOOST_AUTO_TEST_CASE(TestFileReaderInit)
{
    BOOST_CHECK_THROW(FileReader("InvalidFile"), std::exception);

    std::ofstream file(filename);
    file << "AAA";
    file.close();

    FileReader reader(filename);
    FileReader::Buffer buffer;
    BOOST_CHECK(!reader.TryGetLine(&buffer));
    BOOST_CHECK_EQUAL(3, reader.GetFileSize());

    auto chunk = std::make_shared<std::vector<char>>(500);
    BOOST_CHECK(reader.LoadNextChunk(chunk));

    FileReader::Buffer b;
    BOOST_CHECK(reader.TryGetLine(&b));
}

BOOST_AUTO_TEST_CASE(TestFileReaderSimpleRead)
{
    std::ofstream file(filename);
    file << "AAA" << std::endl;
    file << "BBB" << std::endl;
    file << "DDD" << std::endl;
    file.close();

    FileReader reader(filename);
    auto chunk = std::make_shared<std::vector<char>>(500);
    Expect_AAA_BBB_DDD(reader, chunk);
}

BOOST_AUTO_TEST_CASE(TestFileReaderSimpleReadWinEol)
{
    std::ofstream file(filename);
    file << "AAA\r\n";
    file << "BBB\r\n";
    file << "DDD\r\n";
    file.close();

    FileReader reader(filename, "\r\n");
    auto chunk = std::make_shared<std::vector<char>>(500);
    Expect_AAA_BBB_DDD(reader, chunk);
}


BOOST_AUTO_TEST_CASE(TestFileReaderSimpleReadNoLastEol)
{
    std::ofstream file(filename);
    file << "AAA" << std::endl;
    file << "BBB" << std::endl;
    file << "DDD";
    file.close();

    FileReader reader(filename);
    auto chunk = std::make_shared<std::vector<char>>(500);
    Expect_AAA_BBB_DDD(reader, chunk);
}


BOOST_AUTO_TEST_CASE(TestFileReaderOneLineNoEol)
{
    std::ofstream file(filename);
    file << "ABCGHIJK";
    file.close();

    FileReader reader(filename, "\n");
    auto chunk = std::make_shared<std::vector<char>>(500);
    BOOST_CHECK(reader.LoadNextChunk(chunk));

    FileReader::Buffer b;
    BOOST_CHECK(reader.TryGetLine(&b));
    BOOST_CHECK_EQUAL("ABCGHIJK", ToStr(b));

    BOOST_CHECK(!reader.TryGetLine(&b));
    BOOST_CHECK(!reader.LoadNextChunk(chunk));
}

BOOST_AUTO_TEST_CASE(TestFileReaderSimpleReadChunks)
{
    std::ofstream file(filename);
    file << "ABC\n";
    file << "DEF\n";
    file << "GHIJK\n";
    file.close();

    FileReader reader(filename, "\n");
    auto chunk = std::make_shared<std::vector<char>>(6);
    BOOST_CHECK(reader.LoadNextChunk(chunk));

    FileReader::Buffer b;
    BOOST_CHECK(reader.TryGetLine(&b));
    BOOST_CHECK_EQUAL("ABC", ToStr(b));

    BOOST_CHECK(!reader.TryGetLine(&b));
    BOOST_CHECK(!reader.TryGetLine(&b));

    BOOST_CHECK(reader.LoadNextChunk(chunk));

    BOOST_CHECK(reader.TryGetLine(&b));
    BOOST_CHECK_EQUAL("DEF", ToStr(b));

    BOOST_CHECK(!reader.TryGetLine(&b));
    BOOST_CHECK(reader.LoadNextChunk(chunk));

    BOOST_CHECK(reader.TryGetLine(&b));
    BOOST_CHECK_EQUAL("GHIJK", ToStr(b));

    BOOST_CHECK(!reader.TryGetLine(&b));
    BOOST_CHECK(!reader.LoadNextChunk(chunk));
}

BOOST_AUTO_TEST_CASE(TestFileReaderSimpleReadChunksNoLastEol)
{
    std::ofstream file(filename);
    file << "ABC\n";
    file << "DEF\n";
    file << "GHIJK";
    file.close();

    FileReader reader(filename, "\n");
    auto chunk = std::make_shared<std::vector<char>>(6);
    BOOST_CHECK(reader.LoadNextChunk(chunk));

    FileReader::Buffer b;
    BOOST_CHECK(reader.TryGetLine(&b));
    BOOST_CHECK_EQUAL("ABC", ToStr(b));

    BOOST_CHECK(!reader.TryGetLine(&b));
    BOOST_CHECK(!reader.TryGetLine(&b));

    BOOST_CHECK(reader.LoadNextChunk(chunk));

    BOOST_CHECK(reader.TryGetLine(&b));
    BOOST_CHECK_EQUAL("DEF", ToStr(b));

    BOOST_CHECK(!reader.TryGetLine(&b));
    BOOST_CHECK(reader.LoadNextChunk(chunk));

    BOOST_CHECK(reader.TryGetLine(&b));
    BOOST_CHECK_EQUAL("GHIJK", ToStr(b));

    BOOST_CHECK(!reader.TryGetLine(&b));
    BOOST_CHECK(!reader.LoadNextChunk(chunk));
}

BOOST_AUTO_TEST_CASE(TestGetPrefix)
{
    BOOST_CHECK(GetPrefix("ABC", 3) < GetPrefix("BCA", 3));
    BOOST_CHECK(GetPrefix("ABC", 3) < GetPrefix("BC", 2));
    BOOST_CHECK(GetPrefix("BC", 2) == GetPrefix("BC", 2));

    BOOST_CHECK(GetPrefix("ABCDEFGH", 8) == GetPrefix("ABCDEFGH", 8));
    BOOST_CHECK(GetPrefix("ABCDEFGH", 8) < GetPrefix("ZBCDEFGH", 8));
    BOOST_CHECK(GetPrefix("ABCDEFGH", 8) < GetPrefix("AZCDEFGH", 8));
    BOOST_CHECK(GetPrefix("ABCDEFGH", 8) < GetPrefix("ABZDEFGH", 8));
    BOOST_CHECK(GetPrefix("ABCDEFGH", 8) < GetPrefix("ABCZEFGH", 8));
    BOOST_CHECK(GetPrefix("ABCDEFGH", 8) < GetPrefix("ABCDZFGH", 8));
    BOOST_CHECK(GetPrefix("ABCDEFGH", 8) < GetPrefix("ABCDEZGH", 8));
    BOOST_CHECK(GetPrefix("ABCDEFGH", 8) < GetPrefix("ABCDEFZH", 8));
    BOOST_CHECK(GetPrefix("ABCDEFGH", 8) < GetPrefix("ABCDEFHZ", 8));

    BOOST_CHECK(XXH64("ABCDEFGH", 8, 43) == XXH64("ABCDEFGH", 8, 43));
}

template<typename TEntry>
void TestEntryCmp()
{
    BOOST_CHECK(!(TEntry("124. AA", 7) < TEntry("124. AA", 7)));

    // Cmp inside prefix diff by val
    BOOST_CHECK(TEntry("124. ABCDEFG", 13) < TEntry("124. ZBCDEFA", 13));
    BOOST_CHECK(TEntry("124. A", 6) < TEntry("124. B", 6));

    // Cmp inside prefix diff by len
    BOOST_CHECK(TEntry("124. AA", 7) < TEntry("124. AAA", 8));

    // equality
    BOOST_CHECK(!(TEntry("124. AA", 7) < TEntry("124. AA", 7)));
    BOOST_CHECK(!(TEntry("124. ABCDEFG", 13) < TEntry("124. ABCDEFG", 13)));

    // Cmp outside prefix diff by val
    BOOST_CHECK(TEntry("124. AAAAAAAAABCDEFG", 21) < TEntry("124. AAAAAAAAZBCDEFA", 21));
    BOOST_CHECK(TEntry("124. AAAAAAAAA", 14) < TEntry("124. AAAAAAAAB", 14));

    // Cmp outside prefix diff by len
    BOOST_CHECK(TEntry("124. AAAAAAAAAA", 15) < TEntry("124. AAAAAAAAAAA", 16));
    BOOST_CHECK(TEntry("124. AA", 7) < TEntry("124. AAAAAAAAAAA", 16));

    // equality
    BOOST_CHECK(!(TEntry("124. AAAAAAAAAA", 15) < TEntry("124. AAAAAAAAAA", 15)));
    BOOST_CHECK(!(TEntry("124. AAAAAAAAABCDEFG", 21) < TEntry("124. AAAAAAAAABCDEFG", 21)));

    // ignore trash after ending
    BOOST_CHECK(TEntry("123. AAAAAZTRASH", 10) < TEntry("124. AAAAATRASH", 10));
    BOOST_CHECK(!(TEntry("124. AAAAAAAAABCDEFGZTRASH", 21) < TEntry("124. AAAAAAAAABCDEFGTRASH", 21)));

    // cmp by number
    BOOST_CHECK(!(TEntry("5. AAAAAAAAAA", 13) < TEntry("124. AAAAAAAAAA", 13)));

    if (TEntry::UseHash)
    {
        BOOST_CHECK(TEntry("123. AAAAAZTRASH", 10).GetHash() != 0);
        BOOST_CHECK_EQUAL(TEntry("123. AAAAAZTRASH", 10).GetHash(), TEntry("123. AAAAAZTRASH", 10).GetHash());
    }
}


BOOST_AUTO_TEST_CASE(TestFastEntryCmp)
{
    TestEntryCmp<TFastEntry<1, false> >();
    TestEntryCmp<TFastEntry<2, false> >();
    TestEntryCmp<TFastEntry<1, true> >();
    TestEntryCmp<TFastEntry<2, true> >();
}

