cmake_minimum_required(VERSION 3.10)

project(ky-burn-engine)

set(LIB_NAME ky-burn-engine)

option(BUILD_APP "Enable build app project" OFF)

add_subdirectory(lib/xorriso)

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
if(BUILD_APP)
    add_executable(${LIB_NAME}
        ${PUBLIC_INCLUDES}
        ${SRCS}
        ${PROJECT_SOURCE_DIR}/main.cpp
        )
else()
    add_library(${LIB_NAME}  STATIC
                ${PUBLIC_INCLUDES}
                ${SRCS}
    )
endif()


target_link_libraries(${LIB_NAME}
    Qt5::Core
    xorriso
)

target_include_directories(
    ${LIB_NAME}
PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

install(TARGETS ${LIB_NAME} DESTINATION /usr/lib/kylin-burner/)


