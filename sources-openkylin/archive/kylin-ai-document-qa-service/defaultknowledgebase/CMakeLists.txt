cmake_minimum_required(VERSION 3.5)

project(knowledgebase_tool)

set(TARGET knowledgebase_tool)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCE_LIST ${PROJECT_SOURCE_DIR}/service/*.cpp main.cpp)
include_directories(${PROJECT_SOURCE_DIR}/service)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Embedding REQUIRED IMPORTED_TARGET kysdk-coreai-embedding)
pkg_check_modules(VECTORDB REQUIRED IMPORTED_TARGET kysdk-vector-engine-client)
pkg_check_modules(UUID REQUIRED IMPORTED_TARGET uuid)
pkg_check_modules(KYDOCUMENT REQUIRED IMPORTED_TARGET kylin-ai-document-service)
find_package(nlohmann_json REQUIRED)

add_executable(${TARGET} ${SOURCE_LIST})

target_link_libraries(${TARGET}
        PRIVATE PkgConfig::Embedding
        PRIVATE PkgConfig::VECTORDB
        PRIVATE PkgConfig::KYDOCUMENT
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE PkgConfig::UUID)