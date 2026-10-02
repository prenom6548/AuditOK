cmake_minimum_required(VERSION 3.5)
project(baichuan-engine-plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(KylinAiEngine CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)

kylin_ai_add_engine_plugin(baichuan-engine-plugin AI_ENGINE
    baichuanengine.h
    baichuanengine.cpp
)

option(ENABLE_TEST "Build Test" OFF)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

target_link_libraries(baichuan-engine-plugin
    baichuan-nlp-engine
)