cmake_minimum_required(VERSION 3.15)

project(ukui-shell-integration
    VERSION 1.0.0
    DESCRIPTION "UKUI Shell Integration"
    LANGUAGES CXX C
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wno-unused-parameter)

find_package(QT NAMES Qt6 Qt5 CONFIG REQUIRED COMPONENTS WaylandClient)

set(QT_PLUGINS_INSTALL_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/plugins/)

find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED COMPONENTS
    WaylandClient
)

if (QT_VERSION_MAJOR EQUAL "5")
    find_package(Qt5XkbCommonSupport REQUIRED)
endif()

set(PROJECT_WAYLAND_PROTOCOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protocols)

add_subdirectory(src)

# Debug Build Only
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # generate compile_commands.json for clangd
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    # A simple example to show how to use the plugin in your qt project
    add_subdirectory(examples)

    find_program(CLANG_FORMAT clang-format)
    if (CLANG_FORMAT)
        if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git/hooks/pre-commit)
            file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/pre-commit
                DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/.git/hooks
                FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            )
        endif()
    else()
        message(FATAL_ERROR "clang-format not found! It is required to format code.")
    endif()
endif()
