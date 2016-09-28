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

template<typename TEntry>
void ExpectLess(const char* s1, const char* s2, int size1 = -1,  int size2 = -1)
{
    size_t len1 = size1 < 0 ? strlen(s1) : static_cast<size_t>(size1);
    size_t len2 = size2 < 0 ? strlen(s2) : static_cast<size_t>(size2);

    TEntry e1(s1, len1);
    TEntry e2(s2, len2);

    BOOST_CHECK(e1 < e2);
    BOOST_CHECK(!(e2 < e1));
}


#define EXPECT_LESS(s1, s2) \
{\
    TEntry e1(s1, strlen(s1));\
    TEntry e2(s2, strlen(s2));\
    BOOST_CHECK(e1 < e2);\
    BOOST_CHECK(!(e2 < e1));\
}

#define EXPECT_LESS1(s1, s2, len1, len2) \
{\
    TEntry e1(s1, len1);\
    TEntry e2(s2, len2);\
    BOOST_CHECK(e1 < e2);\
    BOOST_CHECK(!(e2 < e1));\
}

#define EXPECT_EQUAL(s1, s2) \
{\
    TEntry e1(s1, strlen(s1));\
    TEntry e2(s2, strlen(s2));\
    BOOST_CHECK(!(e1 < e2));\
    BOOST_CHECK(!(e2 < e1));\
    if (TEntry::UseHash)\
    {\
        BOOST_CHECK(e1.GetHash() != 0);\
        BOOST_CHECK_EQUAL(e1.GetHash(), e2.GetHash());\
    }\
}

#define EXPECT_EQUAL1(s1, s2, len1, len2) \
{\
    TEntry e1(s1, len1);\
    TEntry e2(s2, len2);\
    BOOST_CHECK(!(e1 < e2));\
    BOOST_CHECK(!(e2 < e1));\
    if (TEntry::UseHash)\
    {\
        BOOST_CHECK(e1.GetHash() != 0);\
        BOOST_CHECK_EQUAL(e1.GetHash(), e2.GetHash());\
    }\
}

template<typename TEntry>
void TestEntryCmp()
{
    EXPECT_EQUAL("124. AA", "124. AA");

    // Cmp inside prefix diff by val
    EXPECT_LESS("124. ABCDEFG", "124. ZBCDEFA");
    EXPECT_LESS("124. A", "124. B");

    // Cmp inside prefix diff by len
    EXPECT_LESS("124. AA", "124. AAA");

    // equality
    EXPECT_EQUAL("124. AA", "124. AA");
    EXPECT_EQUAL("124. ABCDEFG", "124. ABCDEFG");

    // Cmp outside prefix diff by val
    EXPECT_LESS("124. AAAAAAAAABCDEFG", "124. AAAAAAAAZBCDEFA");
    EXPECT_LESS("124. AAAAAAAAA", "124. AAAAAAAAB");

    // Cmp outside prefix diff by len
    EXPECT_LESS("124. AAAAAAAAAA", "124. AAAAAAAAAAA");
    EXPECT_LESS("124. AA", "124. AAAAAAAAAAA");

    // equality
    EXPECT_EQUAL("124. AAAAAAAAAA", "124. AAAAAAAAAA");
    EXPECT_EQUAL("124. AAAAAAAAABCDEFG", "124. AAAAAAAAABCDEFG");

    // ignore trash after ending
    EXPECT_LESS1("123. AAAAAZTRASH", "124. AAAAATRASH", 10, 10);
    EXPECT_EQUAL1("124. AAAAAAAAABCDEFGZTRASH", "124. AAAAAAAAABCDEFGTRASH", 20, 20);
    // cmp by number
    EXPECT_LESS("5. AAAAAAAAAA", "124. AAAAAAAAAA");
}


BOOST_AUTO_TEST_CASE(TestFastEntryCmp)
{
    TestEntryCmp<SimpleEntry>();
    TestEntryCmp<SmallEntry>();
    TestEntryCmp<FastEntry>();
}

