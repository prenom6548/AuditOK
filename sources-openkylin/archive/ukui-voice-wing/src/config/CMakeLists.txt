message(STATUS "Building ukui-voice-wing config")

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(Gsettings REQUIRED IMPORTED_TARGET gsettings-qt)

add_library(${PROJECT_NAME}-config STATIC
    configmonitor.cpp
    configmonitor.h
)

set_target_properties(${PROJECT_NAME}-config PROPERTIES
    AUTOMOC ON
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${PROJECT_NAME}-config
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(${PROJECT_NAME}-config
    PUBLIC
        Qt5::Core
        PkgConfig::Gsettings
        ${PROJECT_NAME}-log
)
