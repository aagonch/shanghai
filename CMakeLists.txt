project(shanghai)
cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.54.0 REQUIRED system filesystem unit_test_framework)

include_directories(.)

add_definitions(-std=c++11 -g)

set(COMMON_HEADER_LIST
    ./common/Clock.h)

aux_source_directory(./generator GENERATOR_SRC_LIST)
aux_source_directory(./sorter SORTER_SRC_LIST)
aux_source_directory(./tests TESTS_SRC_LIST)

add_executable(generator ${GENERATOR_SRC_LIST} ${COMMON_HEADER_LIST})
target_link_libraries(generator ${Boost_LIBRARIES})

set(XXHASH_SRC_LIST
    ./xxhash/xxhash.h
    ./xxhash/xxhash.c)

set(SORTER_HEADER_LIST
    ./sorter/SortingEntry.h
    ./sorter/FileReader.h
    ./sorter/Utils.h)

add_executable(sorter ${SORTER_SRC_LIST} ${COMMON_HEADER_LIST} ${SORTER_HEADER_LIST} ${XXHASH_SRC_LIST})
target_link_libraries(sorter ${Boost_LIBRARIES})

add_definitions(-DBOOST_TEST_DYN_LINK)
add_executable(tests ${TESTS_SRC_LIST} ${SORTER_HEADER_LIST} ${XXHASH_SRC_LIST})
target_link_libraries(tests ${Boost_LIBRARIES} boost_unit_test_framework)

