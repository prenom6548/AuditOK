cmake_minimum_required(VERSION 3.5)

project(libkysdk-ai-private LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ai-config)
add_subdirectory(ai-assistant)
add_subdirectory(libkyai-data-management-client)