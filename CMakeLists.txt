cmake_minimum_required(VERSION 3.25)
project(CYK_algorithm)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(CYK_algorithm)

add_executable(Main main.cpp)

target_link_libraries(Main CKYAlgorithm)
