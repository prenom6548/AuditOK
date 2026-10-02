cmake_minimum_required(VERSION 3.16)

project(test LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
)

find_package(GTest)
add_executable(units-test test.cpp)
target_link_libraries(units-test PUBLIC kyai-data-management-client GTest::GTest GTest::Main)
