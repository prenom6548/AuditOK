cmake_minimum_required(VERSION 3.16)

# ============================================================
#
# 构建方式（独立）:
#   cmake -S test -B build_test
#   cmake --build build_test
#   ctest --test-dir build_test --output-on-failure
# ============================================================

project(libkysdk-mcp-complete-validation-tests LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs")
endif()

# 项目根目录
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# ------------------------------------------------------------
# GTest：优先用系统包，回退到 Debian 源码包
# ------------------------------------------------------------
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    if(EXISTS /usr/src/googletest/CMakeLists.txt)
        add_subdirectory(/usr/src/googletest ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
    elseif(EXISTS /usr/src/gtest/CMakeLists.txt)
        add_subdirectory(/usr/src/gtest ${CMAKE_BINARY_DIR}/gtest EXCLUDE_FROM_ALL)
    endif()
endif()

if(TARGET GTest::gtest)
    set(MCP_GTEST_TARGET GTest::gtest)
    set(MCP_GTEST_MAIN_TARGET GTest::gtest_main)
elseif(TARGET GTest::GTest)
    set(MCP_GTEST_TARGET GTest::GTest)
    set(MCP_GTEST_MAIN_TARGET GTest::Main)
elseif(TARGET gtest)
    set(MCP_GTEST_TARGET gtest)
    set(MCP_GTEST_MAIN_TARGET gtest_main)
else()
    message(FATAL_ERROR "GTest not found; install libgtest-dev or googletest-source")
endif()

# ------------------------------------------------------------
# Include 路径（与 tests/CMakeLists.txt 保持一致）
# ------------------------------------------------------------
set(MCP_COMMON_INCLUDE_DIRS
    ${PROJECT_ROOT}/src/common
)

set(MCP_SHARED_INCLUDE_DIRS
    ${PROJECT_ROOT}/src/transport
    ${PROJECT_ROOT}/src/server
    ${PROJECT_ROOT}/src/session
    ${PROJECT_ROOT}/src/net
    ${PROJECT_ROOT}/src/ipc
    ${PROJECT_ROOT}/src/log
)

# ------------------------------------------------------------
# Client 静态库（供客户端预检测试链接）
# ------------------------------------------------------------
set(MCP_CLIENT_SOURCES
    ${PROJECT_ROOT}/src/log/logger.cpp
    ${PROJECT_ROOT}/src/protocol/protocol_core.cpp
    ${PROJECT_ROOT}/src/ipc/pipe_communication.cpp
    ${PROJECT_ROOT}/src/net/netio.cpp
    ${PROJECT_ROOT}/src/transport/transport.cpp
    ${PROJECT_ROOT}/src/session/session_manager.cpp
    ${PROJECT_ROOT}/src/client/client/client.cpp
)

add_library(kysdk_mcp_client_lib_for_complete_tests STATIC
    ${MCP_CLIENT_SOURCES}
)

target_include_directories(kysdk_mcp_client_lib_for_complete_tests
    PUBLIC
        ${PROJECT_ROOT}/include
        ${PROJECT_ROOT}/include/client
        ${PROJECT_ROOT}/include/common
        ${PROJECT_ROOT}/src/net
        ${PROJECT_ROOT}/src/ipc
        ${PROJECT_ROOT}/src/log
        ${PROJECT_ROOT}/src/server
        ${PROJECT_ROOT}/src/session
    PRIVATE
        ${PROJECT_ROOT}/src/client/client
        ${PROJECT_ROOT}/src/client/protocol
        ${MCP_SHARED_INCLUDE_DIRS}
        ${MCP_COMMON_INCLUDE_DIRS}
)

target_link_libraries(kysdk_mcp_client_lib_for_complete_tests
    PRIVATE
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
)

target_compile_definitions(kysdk_mcp_client_lib_for_complete_tests PRIVATE
    MCP_SDK_INTERNAL_BUILD
    CPPHTTPLIB_OPENSSL_SUPPORT
)

# ------------------------------------------------------------
# 测试可执行文件
# ------------------------------------------------------------
set(COMMON_TEST_INCLUDE_DIRS
    ${PROJECT_ROOT}/include
    ${PROJECT_ROOT}/include/client
    ${PROJECT_ROOT}/include/server
    ${PROJECT_ROOT}/include/common
    ${PROJECT_ROOT}/src/server
    ${PROJECT_ROOT}/src/client/client
    ${PROJECT_ROOT}/src/client/protocol
    ${MCP_SHARED_INCLUDE_DIRS}
    ${MCP_COMMON_INCLUDE_DIRS}
)

# 1) Server 端 validateCompleteParams 单元测试（仅头文件，无需 server.cpp）
add_executable(validate_complete_params_gtest
    validate_complete_params_gtest.cpp
)
target_compile_definitions(validate_complete_params_gtest PRIVATE
    MCP_SDK_INTERNAL_BUILD
    CPPHTTPLIB_OPENSSL_SUPPORT
)
target_include_directories(validate_complete_params_gtest PRIVATE
    ${COMMON_TEST_INCLUDE_DIRS}
)
target_link_libraries(validate_complete_params_gtest PRIVATE
    ${MCP_GTEST_TARGET}
    ${MCP_GTEST_MAIN_TARGET}
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)
if(ENABLE_COVERAGE)
    target_link_libraries(validate_complete_params_gtest PRIVATE gcov)
endif()

# 2) Client::complete() 本地预检单元测试
add_executable(client_complete_precheck_gtest
    client_complete_precheck_gtest.cpp
)
target_compile_definitions(client_complete_precheck_gtest PRIVATE
    MCP_SDK_INTERNAL_BUILD
    CPPHTTPLIB_OPENSSL_SUPPORT
)
target_include_directories(client_complete_precheck_gtest PRIVATE
    ${COMMON_TEST_INCLUDE_DIRS}
)
target_link_libraries(client_complete_precheck_gtest PRIVATE
    kysdk_mcp_client_lib_for_complete_tests
    ${MCP_GTEST_TARGET}
    ${MCP_GTEST_MAIN_TARGET}
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)
if(ENABLE_COVERAGE)
    target_link_libraries(client_complete_precheck_gtest PRIVATE gcov)
endif()

# ------------------------------------------------------------
# CTest 注册
# ------------------------------------------------------------
enable_testing()
add_test(NAME validate_complete_params_gtest COMMAND validate_complete_params_gtest)
add_test(NAME client_complete_precheck_gtest COMMAND client_complete_precheck_gtest)
