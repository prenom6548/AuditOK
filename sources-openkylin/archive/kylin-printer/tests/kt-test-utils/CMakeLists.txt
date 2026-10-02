cmake_minimum_required(VERSION 3.5)
project(kt-test-utils)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/cpp-stub
    ${PROJECT_SOURCE_DIR}/cpp-stub-ext
    )


file(GLOB SRC
    "cpp-stub/*"
    "cpp-stub-ext/*"
)

add_library(${PROJECT_NAME} STATIC
    ${SRC}
)
