cmake_minimum_required(VERSION 3.16)
project(libkysdk_mcp VERSION 1.0.0 LANGUAGES CXX)

option(BUILD_EXAMPLES "Build consolidated examples from the repository root" OFF)
set(BUILD_TESTING OFF CACHE BOOL "Build tests")
include(CTest)
option(MCP_ENABLE_CLIENT_TEST_SUITE "Build and register client test suite in root build" ON)
option(MCP_ENABLE_SERVER_TEST_SUITE "Build and register server test suite in root build" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

set(MCP_COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
)

set(MCP_SHARED_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/transport
    ${CMAKE_CURRENT_SOURCE_DIR}/src/server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/session
    ${CMAKE_CURRENT_SOURCE_DIR}/src/net
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ipc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log
)

# ============================================================
# Client library
# ============================================================
set(MCP_CLIENT_SOURCES
    src/log/logger.cpp
    src/protocol/protocol_core.cpp
    src/ipc/pipe_communication.cpp
    src/net/netio.cpp
    src/transport/transport.cpp
    src/session/session_manager.cpp
    src/client/client/client.cpp
)

set(MCP_CLIENT_PUBLIC_HEADERS
    include/client/client.h
    include/common/transport.h
    include/common/message.h
    include/common/protocol.h
    src/common/json.hpp
)

add_library(kysdk_mcp_client SHARED
    ${MCP_CLIENT_SOURCES}
    ${MCP_CLIENT_PUBLIC_HEADERS}
)

set_target_properties(kysdk_mcp_client PROPERTIES
    OUTPUT_NAME "kysdk-mcp-client"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(kysdk_mcp_client
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/client
        ${CMAKE_CURRENT_SOURCE_DIR}/include/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src/net
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ipc
        ${CMAKE_CURRENT_SOURCE_DIR}/src/log
        ${CMAKE_CURRENT_SOURCE_DIR}/src/server
        ${CMAKE_CURRENT_SOURCE_DIR}/src/session
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/client/client
        ${CMAKE_CURRENT_SOURCE_DIR}/src/client/protocol
        ${MCP_SHARED_INCLUDE_DIRS}
        ${MCP_COMMON_INCLUDE_DIRS}
)

target_link_libraries(kysdk_mcp_client
    PRIVATE
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
)

target_compile_definitions(kysdk_mcp_client PRIVATE
    MCP_SDK_INTERNAL_BUILD
    CPPHTTPLIB_OPENSSL_SUPPORT
    $<$<CONFIG:Release>:NDEBUG>
)

# ============================================================
# Server library
# ============================================================
set(MCP_SERVER_SOURCES
    src/log/logger.cpp
    src/session/session_manager.cpp
    src/protocol/protocol_core.cpp
    src/ipc/pipe_communication.cpp
    src/net/netio.cpp
    src/transport/transport.cpp
    src/server/server.cpp
)

set(MCP_SERVER_PUBLIC_HEADERS
    include/common/message.h
    include/common/protocol.h
    include/common/transport.h
    include/server/server.h
)

add_library(kysdk_mcp_server SHARED
    ${MCP_SERVER_SOURCES}
    ${MCP_SERVER_PUBLIC_HEADERS}
)

set_target_properties(kysdk_mcp_server PROPERTIES
    OUTPUT_NAME "kysdk-mcp-server"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(kysdk_mcp_server
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/server
        ${CMAKE_CURRENT_SOURCE_DIR}/include/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src/net
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/server
        ${MCP_SHARED_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/client/client
        ${MCP_COMMON_INCLUDE_DIRS}
)

target_link_libraries(kysdk_mcp_server
    PRIVATE
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
)

target_compile_definitions(kysdk_mcp_server PRIVATE
    MCP_SDK_INTERNAL_BUILD
    CPPHTTPLIB_OPENSSL_SUPPORT
    $<$<CONFIG:Release>:NDEBUG>
)

# ============================================================
# Install
# ============================================================
include(GNUInstallDirs)

install(TARGETS kysdk_mcp_client kysdk_mcp_server
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Public headers
set(MCP_CLIENT_INSTALL_INCLUDEDIR
    "${CMAKE_INSTALL_INCLUDEDIR}/kylin-ai/mcp")
set(MCP_SERVER_INSTALL_INCLUDEDIR
    "${CMAKE_INSTALL_INCLUDEDIR}/kylin-ai/mcp")

install(FILES
    src/common/json.hpp
    DESTINATION ${MCP_CLIENT_INSTALL_INCLUDEDIR}/common
)

install(DIRECTORY
    include/common
    include/client
    include/server
    DESTINATION ${MCP_CLIENT_INSTALL_INCLUDEDIR}
)

# pkg-config files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/kysdk-mcp-client.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/kysdk-mcp-client.pc
    @ONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/kysdk-mcp-server.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/kysdk-mcp-server.pc
    @ONLY
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kysdk-mcp-client.pc
    ${CMAKE_CURRENT_BINARY_DIR}/kysdk-mcp-server.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTING AND MCP_ENABLE_CLIENT_TEST_SUITE)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        if(EXISTS /usr/src/googletest/CMakeLists.txt)
            add_subdirectory(/usr/src/googletest ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
        elseif(EXISTS /usr/src/gtest/CMakeLists.txt)
            add_subdirectory(/usr/src/gtest ${CMAKE_BINARY_DIR}/gtest EXCLUDE_FROM_ALL)
        endif()
    endif()
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GTestCompat.cmake)

    set(MCP_CLIENT_TEST_PRIVATE_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/client
        ${CMAKE_CURRENT_SOURCE_DIR}/include/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src/client/client
        ${CMAKE_CURRENT_SOURCE_DIR}/src/client/protocol
        ${MCP_SHARED_INCLUDE_DIRS}
        ${MCP_COMMON_INCLUDE_DIRS}
    )

    add_executable(client_gtest src/client/test/client_gtest.cpp)
    add_executable(protocol_gtest src/client/test/protocol_gtest.cpp)
    add_executable(transport_test src/client/test/transport_gtest.cpp)
    add_executable(logger_test src/client/test/log_gtest.cpp)
    add_executable(pipe_communication_test src/client/test/pipe_communication_test.cpp)
    add_executable(netio_test src/client/test/netio_gtest.cpp)

    foreach(test_target IN ITEMS client_gtest protocol_gtest transport_test logger_test pipe_communication_test netio_test)
        target_compile_definitions(${test_target} PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
        target_include_directories(${test_target} PRIVATE ${MCP_CLIENT_TEST_PRIVATE_INCLUDE_DIRS})
    endforeach()

    target_link_libraries(client_gtest PRIVATE kysdk_mcp_client ${MCP_GTEST_TARGET} ${MCP_GTEST_MAIN_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(protocol_gtest PRIVATE kysdk_mcp_client ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(transport_test PRIVATE kysdk_mcp_client ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(logger_test PRIVATE kysdk_mcp_client ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(pipe_communication_test PRIVATE kysdk_mcp_client ${MCP_GTEST_TARGET} ${MCP_GTEST_MAIN_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(netio_test PRIVATE kysdk_mcp_client ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

    add_test(NAME client_gtest COMMAND client_gtest)
    add_test(NAME protocol_gtest COMMAND protocol_gtest)
    add_test(NAME transport_test COMMAND transport_test)
    add_test(NAME logger_test COMMAND logger_test)
    add_test(NAME pipe_communication_test COMMAND pipe_communication_test)
    add_test(NAME netio_test COMMAND netio_test)
endif()

if(BUILD_TESTING AND MCP_ENABLE_SERVER_TEST_SUITE)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        if(EXISTS /usr/src/googletest/CMakeLists.txt)
            add_subdirectory(/usr/src/googletest ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
        elseif(EXISTS /usr/src/gtest/CMakeLists.txt)
            add_subdirectory(/usr/src/gtest ${CMAKE_BINARY_DIR}/gtest EXCLUDE_FROM_ALL)
        endif()
    endif()
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GTestCompat.cmake)

    set(MCP_SERVER_TEST_PRIVATE_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/server
        ${CMAKE_CURRENT_SOURCE_DIR}/include/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src/server
        ${MCP_SHARED_INCLUDE_DIRS}
        ${MCP_COMMON_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/server
    )

    add_executable(server_gtest src/server/test/server_gtest.cpp)
    add_executable(logger_gtest src/server/test/logger_gtest.cpp)
    add_executable(netio_gtest src/server/test/netio_gtest.cpp)
    add_executable(pipe_communication_gtest src/server/test/pipe_communication_gtest.cpp)
    add_executable(transport_gtest src/server/test/transport_gtest.cpp)
    add_executable(session_manager_gtest src/server/test/session_manager_gtest.cpp)

    foreach(test_target IN ITEMS server_gtest logger_gtest netio_gtest pipe_communication_gtest transport_gtest session_manager_gtest)
        target_compile_definitions(${test_target} PRIVATE MCP_SDK_INTERNAL_BUILD CPPHTTPLIB_OPENSSL_SUPPORT)
        target_include_directories(${test_target} PRIVATE ${MCP_SERVER_TEST_PRIVATE_INCLUDE_DIRS})
    endforeach()

    # File-system server implementation is used only by server_gtest.
    # Keep it out of released libraries while linking it for tests.
    target_sources(server_gtest PRIVATE
        examples/server/filesystem_server.cpp
    )

    target_link_libraries(server_gtest PRIVATE kysdk_mcp_server ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(logger_gtest PRIVATE kysdk_mcp_server ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(netio_gtest PRIVATE kysdk_mcp_server ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(pipe_communication_gtest PRIVATE kysdk_mcp_server ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(transport_gtest PRIVATE kysdk_mcp_server ${MCP_GTEST_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
    target_link_libraries(session_manager_gtest PRIVATE kysdk_mcp_server ${MCP_GTEST_TARGET} ${MCP_GTEST_MAIN_TARGET} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

    add_test(NAME server_gtest COMMAND server_gtest)
    add_test(NAME logger_gtest COMMAND logger_gtest)
    add_test(NAME netio_gtest COMMAND netio_gtest)
    add_test(NAME pipe_communication_gtest COMMAND pipe_communication_gtest)
    add_test(NAME transport_gtest COMMAND transport_gtest)
    add_test(NAME session_manager_gtest COMMAND session_manager_gtest)
endif()
