cmake_minimum_required(VERSION 3.1.3)

project(kba)

include(CMakePackageConfigHelpers)
include(FindPkgConfig)
pkg_check_modules(PKGS REQUIRED
	json-c
	)
include_directories(${PKGS_INCLUDE_DIRS})
include_directories(${PROJECT_BINARY_DIR})

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
	add_definitions(-DRELEASE_BUYILD)
	set(CMAKE_C_FLAGS "$ENV{CFLAGS} -O2 -Wall")
	set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O2 -Wall")
else (CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
	message("CMake Build Type: ${CMAKE_BUILD_TYPE}")
	set(CMAKE_C_FLAGS "$ENV{CFLAGS} -O0 -g -Wall")
	set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O0 -g -Wall")
endif()

message("Build Type: ${CMAKE_BUILD_TYPE}")

if (SYSTEM_VERSION)
	add_definitions(-D${SYSTEM_VERSION})
endif()

set(CMAKE_INSTALL_PREFIX /usr)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(lib)
add_subdirectory(example)
add_subdirectory(kpr-backend)

set(CMAKE_VERBOSE_MAKEFILE ON)

