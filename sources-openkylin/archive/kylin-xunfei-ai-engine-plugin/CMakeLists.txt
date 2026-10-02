cmake_minimum_required(VERSION 3.5)
project(xunfei-engine-plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(KylinAiEngine CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)

include_directories(third-party/cpr/include)

kylin_ai_add_engine_plugin(xunfei-engine-plugin AI_ENGINE
    xunfeiengine.h
    xunfeiengine.cpp
)

option(ENABLE_TEST "Build Test" OFF)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

target_link_libraries(xunfei-engine-plugin
    jsoncpp
    xunfei-speech-engine
    xunfei-vision-engine
    xunfei-nlp-engine
)