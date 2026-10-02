message(STATUS "Building ukui-voice-wing log")

find_package(Qt5 COMPONENTS Core REQUIRED)

add_library(${PROJECT_NAME}-log STATIC
    logger.cpp
    logger.h
)

target_include_directories(${PROJECT_NAME}-log
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(${PROJECT_NAME}-log
    PUBLIC
        Qt5::Core
)
