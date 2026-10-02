cmake_minimum_required(VERSION 3.5)

project(kylin-traditional-engine-plugin)

set(TRAGET kylin-traditional-engine-plugin)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(KylinAiEngine CONFIG REQUIRED)

kylin_ai_add_engine_plugin(kylin-traditional-engine-plugin AI_ENGINE
    embeddingengineprivate.h
    embeddingengineprivate.cpp
    embeddingplugin.h
    embeddingplugin.cpp
)

target_link_libraries(${TRAGET}
    kylin-ondevice-embedding-engine
)

if (DEFINED ENABLE_TEST)
    unset(ENABLE_TEST CACHE)
endif(DEFINED ENABLE_TEST)
option(ENABLE_TEST "Build Test" OFF)
if (ENABLE_TEST)
    add_subdirectory(tests)
endif (ENABLE_TEST)