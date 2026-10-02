cmake_minimum_required(VERSION 3.5)

project(UnitTest)

set(TEST_TARGET UnitTest)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCE_LIST
        ../src/aiIndex/*.cpp
        ../src/controller/*.cpp
        ../src/dao/*.cpp
        ../src/service/*.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(BUSINESS_FRAMEWORK REQUIRED IMPORTED_TARGET kyai-business-framework)
pkg_check_modules(DOCUMENT_SERVICE REQUIRED IMPORTED_TARGET kylin-ai-document-service)
pkg_check_modules(VECTOR_DB REQUIRED IMPORTED_TARGET kysdk-vector-engine-client)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(UUID REQUIRED IMPORTED_TARGET uuid)
pkg_check_modules(NLP REQUIRED IMPORTED_TARGET kysdk-genai-nlp)

find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED)

find_package(SQLite3 REQUIRED)
if(SQLite3_VERSION VERSION_GREATER_EQUAL "3.42.0")
    message(STATUS "SQLite3 version ${SQLite3_VERSION} >= 3.42.0 - Enabling database recovery features")
    set(SQLITE_RECOVERY_SOURCES
        ../src/sqliteRecover/dbdata.c
        ../src/sqliteRecover/sqlite3recover.c
        ../src/sqliteRecover/recoverdb.cpp)
else()
    message(WARNING "SQLite3 version ${SQLite3_VERSION} is too old (need >= 3.42.0) - Disabling database recovery features")
    set(SQLITE_RECOVERY_SOURCES "")
endif()

include_directories(
        ../src
        ../src/service
        ../src/dao
        ../src/aiIndex
        ../src/sqliteRecover
        ${GIO_INCLUDE_DIRS}
        ${NLP_INCLUDE_DIRS}
        )

add_executable(${TEST_TARGET} ${SOURCE_LIST} ${SQLITE_RECOVERY_SOURCES}
        dao/testVector.cpp
        dao/testEmbedding.cpp 
        dao/testDocumentParse.cpp
        dao/testFileDatabase.cpp
        service/testTagService.cpp
        service/testImageService.cpp
        service/testfileInfoService.cpp
        testController.cpp
        aiIndex/testTextEmbeddingTask.cpp
        aiIndex/testBaseTask.cpp
        aiIndex/testTagTask.cpp
        aiIndex/testSummaryTask.cpp
        aiIndex/testAIIndex.cpp
        )

if(SQLite3_VERSION VERSION_GREATER_EQUAL "3.42.0")
        target_compile_definitions(${TEST_TARGET} PRIVATE ENABLE_SQLITE_DB_RECOVERY)
endif()

target_link_libraries(${TEST_TARGET} 
        PRIVATE GTest::GTest GTest::Main
        PRIVATE PkgConfig::VECTOR_DB
        PRIVATE PkgConfig::BUSINESS_FRAMEWORK
        PRIVATE PkgConfig::DOCUMENT_SERVICE
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE PkgConfig::UUID
        PRIVATE PkgConfig::NLP
        PRIVATE ${GIO_LIBRARIES}
        PRIVATE pthread
        PRIVATE ${SQLite3_LIBRARIES}
        )

# 解决龙芯架构不支持 fstream 标准库问题
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(${TEST_TARGET} PRIVATE stdc++fs)
    endif ()
endif ()

add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})