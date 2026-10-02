cmake_minimum_required(VERSION 3.16)

project(libkysdk-mcp-tests LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable coverage if requested
option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs")
endif()

# Get the project root directory
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Find or fetch GTest
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    if(EXISTS /usr/src/googletest/CMakeLists.txt)
        add_subdirectory(/usr/src/googletest ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
    elseif(EXISTS /usr/src/gtest/CMakeLists.txt)
        add_subdirectory(/usr/src/gtest ${CMAKE_BINARY_DIR}/gtest EXCLUDE_FROM_ALL)
    endif()
endif()

# GTest target compatibility
if(TARGET GTest::gtest)
    set(MCP_GTEST_TARGET GTest::gtest)
    set(MCP_GTEST_MAIN_TARGET GTest::gtest_main)
elseif(TARGET GTest::GTest)
    set(MCP_GTEST_TARGET GTest::GTest)
    set(MCP_GTEST_MAIN_TARGET GTest::Main)
elseif(TARGET gtest)
    set(MCP_GTEST_TARGET gtest)
    set(MCP_GTEST_MAIN_TARGET gtest_main)
endif()

# Common include directories
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

# ============================================================
# Build Client library for tests
# ============================================================
set(MCP_CLIENT_SOURCES
    ${PROJECT_ROOT}/src/log/logger.cpp
    ${PROJECT_ROOT}/src/protocol/protocol_core.cpp
    ${PROJECT_ROOT}/src/ipc/pipe_communication.cpp
    ${PROJECT_ROOT}/src/net/netio.cpp
    ${PROJECT_ROOT}/src/transport/transport.cpp
    ${PROJECT_ROOT}/src/session/session_manager.cpp
    ${PROJECT_ROOT}/src/client/client/client.cpp
)

add_library(kysdk_mcp_client_lib STATIC
    ${MCP_CLIENT_SOURCES}
)

target_include_directories(kysdk_mcp_client_lib
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

target_link_libraries(kysdk_mcp_client_lib
    PRIVATE
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
)

target_compile_definitions(kysdk_mcp_client_lib PRIVATE
    MCP_SDK_INTERNAL_BUILD
    CPPHTTPLIB_OPENSSL_SUPPORT
)

# ============================================================
# Build Server library for tests
# ============================================================
set(MCP_SERVER_SOURCES
    ${PROJECT_ROOT}/src/log/logger.cpp
    ${PROJECT_ROOT}/src/session/session_manager.cpp
    ${PROJECT_ROOT}/src/protocol/protocol_core.cpp
    ${PROJECT_ROOT}/src/ipc/pipe_communication.cpp
    ${PROJECT_ROOT}/src/net/netio.cpp
    ${PROJECT_ROOT}/src/transport/transport.cpp
    ${PROJECT_ROOT}/src/server/server.cpp
)

add_library(kysdk_mcp_server_lib STATIC
    ${MCP_SERVER_SOURCES}
)

target_include_directories(kysdk_mcp_server_lib
    PUBLIC
        ${PROJECT_ROOT}/include
        ${PROJECT_ROOT}/include/server
        ${PROJECT_ROOT}/include/common
        ${PROJECT_ROOT}/src/net
    PRIVATE
        ${PROJECT_ROOT}/src/server
        ${MCP_SHARED_INCLUDE_DIRS}
        ${PROJECT_ROOT}/src/client/client
        ${MCP_COMMON_INCLUDE_DIRS}
)

target_link_libraries(kysdk_mcp_server_lib
    PRIVATE
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
)

target_compile_definitions(kysdk_mcp_server_lib PRIVATE
    MCP_SDK_INTERNAL_BUILD
    CPPHTTPLIB_OPENSSL_SUPPORT
)

# ============================================================
# Client Tests
# ============================================================
set(MCP_CLIENT_TEST_PRIVATE_INCLUDE_DIRS
    ${PROJECT_ROOT}/include
    ${PROJECT_ROOT}/include/client
    ${PROJECT_ROOT}/include/common
    ${PROJECT_ROOT}/src/client/client
    ${PROJECT_ROOT}/src/client/protocol
    ${MCP_SHARED_INCLUDE_DIRS}
    ${MCP_COMMON_INCLUDE_DIRS}
)

# File-system server implementation is used only by client_gtest
set(CLIENT_TEST_SERVER_SOURCES
    ${PROJECT_ROOT}/examples/server/filesystem_server.cpp
)

add_executable(client_gtest
    client/client_gtest.cpp
)
target_compile_definitions(client_gtest PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(client_gtest PRIVATE 
    ${MCP_CLIENT_TEST_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_ROOT}/examples/server
)
target_link_libraries(client_gtest PRIVATE kysdk_mcp_client_lib ${MCP_GTEST_TARGET} ${MCP_GTEST_MAIN_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(client_gtest PRIVATE gcov)
endif()

add_executable(protocol_gtest client/protocol_gtest.cpp)
target_compile_definitions(protocol_gtest PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(protocol_gtest PRIVATE ${MCP_CLIENT_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(protocol_gtest PRIVATE kysdk_mcp_client_lib ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(protocol_gtest PRIVATE gcov)
endif()

add_executable(transport_test client/transport_gtest.cpp)
target_compile_definitions(transport_test PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(transport_test PRIVATE ${MCP_CLIENT_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(transport_test PRIVATE kysdk_mcp_client_lib ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(transport_test PRIVATE gcov)
endif()

add_executable(logger_test client/log_gtest.cpp)
target_compile_definitions(logger_test PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(logger_test PRIVATE ${MCP_CLIENT_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(logger_test PRIVATE kysdk_mcp_client_lib ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(logger_test PRIVATE gcov)
endif()

add_executable(pipe_communication_test client/pipe_communication_test.cpp)
target_compile_definitions(pipe_communication_test PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(pipe_communication_test PRIVATE ${MCP_CLIENT_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(pipe_communication_test PRIVATE kysdk_mcp_client_lib ${MCP_GTEST_TARGET} ${MCP_GTEST_MAIN_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(pipe_communication_test PRIVATE gcov)
endif()

add_executable(netio_test client/netio_gtest.cpp)
target_compile_definitions(netio_test PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(netio_test PRIVATE ${MCP_CLIENT_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(netio_test PRIVATE kysdk_mcp_client_lib ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(netio_test PRIVATE gcov)
endif()

# ============================================================
# Server Tests
# ============================================================
set(MCP_SERVER_TEST_PRIVATE_INCLUDE_DIRS
    ${PROJECT_ROOT}/include
    ${PROJECT_ROOT}/include/server
    ${PROJECT_ROOT}/include/common
    ${PROJECT_ROOT}/src/server
    ${MCP_SHARED_INCLUDE_DIRS}
    ${MCP_COMMON_INCLUDE_DIRS}
    ${PROJECT_ROOT}/examples/server
)

add_executable(server_gtest 
    server/server_gtest.cpp
    ${PROJECT_ROOT}/examples/server/filesystem_server.cpp
)
target_compile_definitions(server_gtest PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(server_gtest PRIVATE ${MCP_SERVER_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(server_gtest PRIVATE kysdk_mcp_server_lib ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(server_gtest PRIVATE gcov)
endif()

add_executable(server_logger_gtest server/logger_gtest.cpp)
target_compile_definitions(server_logger_gtest PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(server_logger_gtest PRIVATE ${MCP_SERVER_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(server_logger_gtest PRIVATE kysdk_mcp_server_lib ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(server_logger_gtest PRIVATE gcov)
endif()

add_executable(server_netio_gtest server/netio_gtest.cpp)
target_compile_definitions(server_netio_gtest PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(server_netio_gtest PRIVATE ${MCP_SERVER_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(server_netio_gtest PRIVATE kysdk_mcp_server_lib ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(server_netio_gtest PRIVATE gcov)
endif()

add_executable(pipe_communication_gtest server/pipe_communication_gtest.cpp)
target_compile_definitions(pipe_communication_gtest PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(pipe_communication_gtest PRIVATE ${MCP_SERVER_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(pipe_communication_gtest PRIVATE kysdk_mcp_server_lib ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(pipe_communication_gtest PRIVATE gcov)
endif()

add_executable(server_transport_gtest server/transport_gtest.cpp)
target_compile_definitions(server_transport_gtest PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(server_transport_gtest PRIVATE ${MCP_SERVER_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(server_transport_gtest PRIVATE kysdk_mcp_server_lib ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(server_transport_gtest PRIVATE gcov)
endif()

add_executable(session_manager_gtest server/session_manager_gtest.cpp)
target_compile_definitions(session_manager_gtest PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(session_manager_gtest PRIVATE ${MCP_SERVER_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(session_manager_gtest PRIVATE kysdk_mcp_server_lib ${MCP_GTEST_TARGET} ${MCP_GTEST_MAIN_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(session_manager_gtest PRIVATE gcov)
endif()

# Regression: ensure IPC server start/stop never deadlocks (see bug/start_01.cpp).
add_executable(server_start_stop_no_deadlock_gtest server/server_start_stop_no_deadlock_gtest.cpp)
target_compile_definitions(server_start_stop_no_deadlock_gtest PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
target_include_directories(server_start_stop_no_deadlock_gtest PRIVATE ${MCP_SERVER_TEST_PRIVATE_INCLUDE_DIRS})
target_link_libraries(server_start_stop_no_deadlock_gtest PRIVATE kysdk_mcp_server_lib ${MCP_GTEST_TARGET} ${MCP_GTEST_MAIN_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
if(ENABLE_COVERAGE)
    target_link_libraries(server_start_stop_no_deadlock_gtest PRIVATE gcov)
endif()

# ============================================================
# CTest configuration
# ============================================================
enable_testing()

# Client tests
add_test(NAME client_gtest COMMAND client_gtest)
add_test(NAME protocol_gtest COMMAND protocol_gtest)
add_test(NAME transport_test COMMAND transport_test)
add_test(NAME logger_test COMMAND logger_test)
add_test(NAME pipe_communication_test COMMAND pipe_communication_test)
add_test(NAME netio_test COMMAND netio_test)

# Server tests
add_test(NAME server_gtest COMMAND server_gtest)
add_test(NAME server_logger_gtest COMMAND server_logger_gtest)
add_test(NAME server_netio_gtest COMMAND server_netio_gtest)
add_test(NAME pipe_communication_gtest COMMAND pipe_communication_gtest)
add_test(NAME server_transport_gtest COMMAND server_transport_gtest)
add_test(NAME session_manager_gtest COMMAND session_manager_gtest)
add_test(NAME server_start_stop_no_deadlock_gtest COMMAND server_start_stop_no_deadlock_gtest)
set_tests_properties(server_start_stop_no_deadlock_gtest PROPERTIES TIMEOUT 30)
