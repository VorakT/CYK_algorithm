cmake_minimum_required(VERSION 3.25)
project(CYK_algorithm)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)

enable_testing()

set(CMAKE_CXX_FLAGS "--coverage")

include_directories(../CYK_algorithm)

add_subdirectory(../CYK_algorithm build)  

add_executable(Test CKYAlgorithmTestCase.cpp)

include(GoogleTest)
find_package(GTest REQUIRED)

target_link_libraries(Test CKYAlgorithm)
target_link_libraries(Test GTest::GTest GTest::Main)

gtest_discover_tests(Test)

