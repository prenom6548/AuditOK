cmake_minimum_required(VERSION 3.16)
project(ukui-menu-extensions)

if(BUILD_TEST)
    enable_testing()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

add_subdirectory(recent-file)
add_subdirectory(ernie-bot)
