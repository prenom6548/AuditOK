cmake_minimum_required(VERSION 3.0.0)
project(simple_logger)

include(CheckIncludeFileCXX)

set(CMAKE_CXX_STANDARD 20)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  MESSAGE("#### Use Clang compiler ####")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  MESSAGE("#### Use GNU compiler ####")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  MESSAGE("#### Use Intel compiler ####")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  MESSAGE("#### Use MSVC compiler ####")
endif()

CHECK_INCLUDE_FILE_CXX(format HAS_FORMAT)
message(HAS_FORMAT=${HAS_FORMAT})

if(${HAS_FORMAT})
    message("HAS_FORMAT=${HAS_FORMAT}, add HAS_STD_FORMAT definition")
    add_definitions(-DHAS_STD_FORMAT)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../lib)

set(SRC 
    ${PROJECT_SOURCE_DIR}/src/DateTime.cpp
    ${PROJECT_SOURCE_DIR}/src/Formatter.cpp
    ${PROJECT_SOURCE_DIR}/src/Logger.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

link_directories(${PROJECT_SOURCE_DIR})

#add_library(simple_logger STATIC ${SRC})
add_library(simple_logger SHARED ${SRC})

if(WIN32)
    MESSAGE(STATUS "Current OS is windows system")
    target_link_libraries(simple_logger)
elseif(APPLE)
    MESSAGE(STATUS "Current OS is Apple system.")
    target_link_libraries(simple_logger -lstdc++ -lpthread)
elseif(UNIX)
    MESSAGE(STATUS "Current OS is UNIX-like(including linux) system.")
    target_link_libraries(simple_logger -lstdc++ -lpthread)
endif()

