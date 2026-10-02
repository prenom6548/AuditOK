cmake_minimum_required(VERSION 3.12)

project(libkylin-ai-system-private VERSION 1.0.0)

option(KYLIN_BUILD_AI_ENGINE "Build ai-engine library" ON)
option(KYLIN_BUILD_AI_PROTO "Build ai-proto definitions" ON)

if(KYLIN_BUILD_AI_ENGINE)
    add_subdirectory(ai-engine)
endif()

if(KYLIN_BUILD_AI_PROTO)
    add_subdirectory(ai-proto)
endif()
