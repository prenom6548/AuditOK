cmake_minimum_required(VERSION 3.5)
project(baidu-engine-plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(KylinAiEngine CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)

include_directories(third-party/cpr/include)

kylin_ai_add_engine_plugin(baidu-engine-plugin AI_ENGINE
    baiduengine.h
    baiduengine.cpp
)

option(ENABLE_TEST "Build Test" OFF)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

target_link_libraries(baidu-engine-plugin
    jsoncpp
    baidu-nlp-engine
    baidu-speech-engine
    baidu-vision-engine
)