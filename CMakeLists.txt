cmake_minimum_required(VERSION 3.17)
project(bitset_mpsi)

# Set global variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

# Load dependencies
find_package(GMP REQUIRED)
find_package(NTL REQUIRED)

# Libraries
add_library(ThresholdPaillier threshold_paillier.h threshold_paillier.cpp)
add_library(BloomFilter bloom_filter.h bloom_filter.cpp)
add_library(PSIProtocols psi_protocols.h psi_protocols.cpp)
add_library(SubProtocols sub_protocols.h sub_protocols.cpp)
add_library(Benchmarking benchmarking.h benchmarking.cpp)

# Bundle everything together
add_executable(bitset_mpsi main.cpp)
target_include_directories(bitset_mpsi PUBLIC ${NTL_INCLUDE_DIR} ${GMP_INCLUDE_DIR})
target_link_libraries(bitset_mpsi
        Benchmarking
        PSIProtocols
        SubProtocols
        BloomFilter
        ThresholdPaillier
        ${NTL_LIBRARY}
        ${GMP_LIBRARIES})
