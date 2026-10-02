cmake_minimum_required(VERSION 3.5)

project(ky-udf-tools LANGUAGES CXX)
set(LIB_NAME ky-udf-tools)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

# Setup the environment
find_package(Qt5 COMPONENTS Core REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# public include
file(GLOB_RECURSE PUBLIC_INCLUDES  CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*")

# src
FILE (GLOB_RECURSE SRCS CONFIGURE_DEPENDS
    "./src/*"
)

# Build
add_library(${LIB_NAME}  STATIC
            ${PUBLIC_INCLUDES}
            ${SRCS}
)


target_include_directories(
    ${LIB_NAME}
PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${LIB_NAME}
    Qt5::Core
)



