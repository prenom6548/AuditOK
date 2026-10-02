cmake_minimum_required(VERSION 3.16)

project(kyai-data-management-client LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-unix-2.0)
find_package(nlohmann_json REQUIRED)
find_package(KylinAiProto REQUIRED)
kylin_ai_generate_gdbus_proto_code(DATA_MANAGEMENT_PROTO_FILES --glib-min-required=2.64 datamanagementservice)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${GIO_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCE_FLIES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(kyai-data-management-client SHARED
        ${SOURCE_FLIES}
        ${DATA_MANAGEMENT_PROTO_FILES})
set_target_properties(kyai-data-management-client PROPERTIES VERSION 1.0.0 SOVERSION 1)

target_link_libraries(kyai-data-management-client
        PRIVATE PkgConfig::GIO
        PRIVATE nlohmann_json::nlohmann_json)

# 解决龙芯架构不支持 fstream 标准库问题
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        target_link_libraries(kyai-data-management-client PRIVATE stdc++fs)
    endif ()
endif ()

install(TARGETS kyai-data-management-client
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/
        DESTINATION include)
install(FILES pkgconf/kyai-data-management-client.pc
        DESTINATION /usr/share/pkgconfig)

if (DEFINED ENABLE_TEST)
    unset(ENABLE_TEST CACHE)
endif(DEFINED ENABLE_TEST)
option(ENABLE_TEST "Build Test" OFF)
if (ENABLE_TEST)
    add_subdirectory(test)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif (ENABLE_TEST)
