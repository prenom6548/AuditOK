cmake_minimum_required(VERSION 3.10)
project(test LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(TARGET test)

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
)

add_executable(${TARGET}
    test.cpp
)


target_link_libraries(${TARGET}
    PRIVATE kylin-ai-document-service
)