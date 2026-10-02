cmake_minimum_required(VERSION 3.5)

project(kyai-data-management-service LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_LIST
    ${PROJECT_SOURCE_DIR}/src/aiIndex/*.cpp
    ${PROJECT_SOURCE_DIR}/src/controller/*.cpp
    ${PROJECT_SOURCE_DIR}/src/dao/*.cpp
    ${PROJECT_SOURCE_DIR}/src/service/*.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(VECTOR_DB REQUIRED IMPORTED_TARGET kysdk-vector-engine-client)
pkg_check_modules(BUSINESS_FRAMEWORK REQUIRED IMPORTED_TARGET kyai-business-framework)
pkg_check_modules(DOCUMENT_SERVICE REQUIRED IMPORTED_TARGET kylin-ai-document-service)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(NLP REQUIRED IMPORTED_TARGET kysdk-genai-nlp)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)
if(SQLite3_VERSION VERSION_GREATER_EQUAL "3.42.0")
    message(STATUS "SQLite3 version ${SQLite3_VERSION} >= 3.42.0 - Enabling database recovery features")
    set(SQLITE_RECOVERY_SOURCES
        ${PROJECT_SOURCE_DIR}/src/sqliteRecover/dbdata.c
        ${PROJECT_SOURCE_DIR}/src/sqliteRecover/sqlite3recover.c
        ${PROJECT_SOURCE_DIR}/src/sqliteRecover/recoverdb.cpp)
else()
    message(WARNING "SQLite3 version ${SQLite3_VERSION} is too old (need >= 3.42.0) - Disabling database recovery features")
    set(SQLITE_RECOVERY_SOURCES "")
endif()

include_directories(${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/service
        ${PROJECT_SOURCE_DIR}/src/dao
        ${PROJECT_SOURCE_DIR}/src/aiIndex
        ${PROJECT_SOURCE_DIR}/src/sqliteRecover
        ${GIO_INCLUDE_DIRS}
        ${NLP_INCLUDE_DIRS})

add_executable(kyai-data-management-service ${SOURCE_LIST} ${SQLITE_RECOVERY_SOURCES})

if(SQLite3_VERSION VERSION_GREATER_EQUAL "3.42.0")
    target_compile_definitions(kyai-data-management-service PRIVATE ENABLE_SQLITE_DB_RECOVERY)
endif()

target_link_libraries(kyai-data-management-service
        PRIVATE PkgConfig::BUSINESS_FRAMEWORK
        PRIVATE PkgConfig::VECTOR_DB
        PRIVATE PkgConfig::DOCUMENT_SERVICE
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE PkgConfig::NLP
        PRIVATE ${SQLite3_LIBRARIES}
        PRIVATE ${GIO_LIBRARIES}
        PRIVATE pthread)

# 解决龙芯架构不支持 fstream 标准库问题
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(kyai-data-management-service PRIVATE stdc++fs)
    endif ()
endif ()

install(FILES conf/DataManagement.json DESTINATION /usr/share/kylin-ai/kyai-business-framework)
install(FILES conf/kyai-data-management-service.service DESTINATION /usr/lib/systemd/user)
install(TARGETS kyai-data-management-service
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if (DEFINED ENABLE_TEST)
    unset(ENABLE_TEST CACHE)
endif (DEFINED ENABLE_TEST)
option(ENABLE_TEST "Build Test" OFF)
if (BUILD_TESTING OR ENABLE_TEST)
    # 单元测试覆盖率 本地获取覆盖率报告即可 获取前需要sudo apt install lcov 
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    enable_testing()
    add_subdirectory(test)
endif ()
