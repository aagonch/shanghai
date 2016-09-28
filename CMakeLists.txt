project(shanghai)
cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.54.0)

include_directories(.)

#add_definitions(-std=c++11 -g -D_GLIBCXX_USE_CXX11_ABI=0)

set(COMMON_HEADER_LIST
    ./common/Clock.h
    ./common/Utils.h)

aux_source_directory(./generator GENERATOR_SRC_LIST)
aux_source_directory(./sorter SORTER_SRC_LIST)
aux_source_directory(./tests TESTS_SRC_LIST)

add_executable(generator ${GENERATOR_SRC_LIST} ${COMMON_HEADER_LIST})
target_link_libraries(generator ${Boost_LIBRARIES})

set(SORTER_HEADER_LIST
    ./sorter/SortingEntry.h
    ./sorter/FileReader.h
    ./sorter/FileRegistry.h
    ./sorter/FileWriter.h
    ./sorter/InitialSorter.h
    ./sorter/Merger.h
    )

add_executable(sorter ${SORTER_SRC_LIST} ${COMMON_HEADER_LIST} ${SORTER_HEADER_LIST})
target_link_libraries(sorter ${Boost_LIBRARIES})

add_definitions(-DBOOST_TEST_DYN_LINK)
add_executable(tests ${TESTS_SRC_LIST} ${SORTER_HEADER_LIST})
target_link_libraries(tests ${Boost_LIBRARIES} boost_unit_test_framework)

