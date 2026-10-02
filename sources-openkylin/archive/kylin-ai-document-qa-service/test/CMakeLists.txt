cmake_minimum_required(VERSION 3.5)

project(test LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-unix-2.0)
pkg_check_modules(KnowledgeBase REQUIRED IMPORTED_TARGET kyai-knowledgebase-client)

# 单元测试
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

add_executable(test main.cpp)

target_link_libraries(test 
            PUBLIC PkgConfig::KnowledgeBase
            PUBLIC PkgConfig::GIO
            PUBLIC kyai-business-framework
            )