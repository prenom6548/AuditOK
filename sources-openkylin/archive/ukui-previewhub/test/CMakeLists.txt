# Standalone self-test binary — not part of parent ukui-previewhub CMake / deb package.
cmake_minimum_required(VERSION 3.16)
project(ukui-previewhub-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(_HUB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(UKUI_PREVIEWHUB_BUILD_DIR "${_HUB_ROOT}/build" CACHE PATH
    "Directory with locally built libukui-previewhub.so (parent cmake -B …). Ignored if empty or library not found there.")

set(_UKUI_LIB_FROM_BUILD FALSE)
if(UKUI_PREVIEWHUB_BUILD_DIR)
    find_library(_UKUI_PREVIEWHUB_LIB
        NAMES ukui-previewhub libukui-previewhub.so.1 libukui-previewhub.so
        PATHS "${UKUI_PREVIEWHUB_BUILD_DIR}"
        NO_DEFAULT_PATH
    )
    if(_UKUI_PREVIEWHUB_LIB)
        set(_UKUI_LIB_FROM_BUILD TRUE)
    endif()
endif()

if(NOT _UKUI_PREVIEWHUB_LIB)
    # Installed package: /usr/lib/<triplet>/libukui-previewhub.so*
    find_library(_UKUI_PREVIEWHUB_LIB
        NAMES ukui-previewhub libukui-previewhub.so.1 libukui-previewhub.so
    )
endif()

if(NOT _UKUI_PREVIEWHUB_LIB)
    message(FATAL_ERROR
        "libukui-previewhub not found.\n"
        "  A) Local build: cmake -S ${_HUB_ROOT} -B ${_HUB_ROOT}/build && cmake --build ${_HUB_ROOT}/build\n"
        "     then: cmake -S . -B build -DUKUI_PREVIEWHUB_BUILD_DIR=${_HUB_ROOT}/build\n"
        "  B) System package: install libukui-previewhub1 (and headers under /usr/include/ukui-previewhub), "
        "then re-run cmake here (library is searched in default linker paths).")
endif()

# Headers: tree checkout ../include, or paths used by the .deb
set(_API_INC "${_HUB_ROOT}/include")
if(NOT EXISTS "${_API_INC}/ukui_previewhub_api.h")
    find_path(_API_INC ukui_previewhub_api.h
        PATH_SUFFIXES ukui-previewhub
        PATHS /usr/local/include /usr/include
    )
endif()
if(NOT EXISTS "${_API_INC}/ukui_previewhub_api.h")
    message(FATAL_ERROR
        "ukui_previewhub_api.h not found. Install development files from the same package that ships headers, "
        "or build from the full ukui-previewhub source tree (../include).")
endif()

add_executable(ukui-previewhub-test main.cpp)
target_include_directories(ukui-previewhub-test PRIVATE "${_API_INC}")
target_link_libraries(ukui-previewhub-test PRIVATE Qt5::Core Qt5::Widgets "${_UKUI_PREVIEWHUB_LIB}")

if(UNIX AND NOT APPLE AND _UKUI_LIB_FROM_BUILD)
    set_target_properties(ukui-previewhub-test PROPERTIES
        BUILD_RPATH "${UKUI_PREVIEWHUB_BUILD_DIR}"
        INSTALL_RPATH ""
    )
endif()
