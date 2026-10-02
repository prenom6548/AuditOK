cmake_minimum_required(VERSION 3.5)

project(demo)

set(TARGET demo)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/../
)

find_package(nlohmann_json REQUIRED)

set(SOURCE_FILES
    test.cpp
)

add_executable(${TARGET} ${SOURCE_FILES})

target_link_libraries(${TARGET}
    PRIVATE kylin-ondevice-embedding-engine
    PRIVATE nlohmann_json::nlohmann_json
)