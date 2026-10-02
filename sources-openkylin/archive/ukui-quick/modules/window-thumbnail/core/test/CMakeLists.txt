cmake_minimum_required(VERSION 3.16)

project(thumbnail-item-test)

if(NOT DEFINED ROOT_PROJECT_NAME)
    set(ROOT_PROJECT_NAME ukui-quick)
endif()
if(NOT DEFINED VERSION_MAJOR)
    set(VERSION_MAJOR 1)
endif()
if(NOT DEFINED VERSION_MINOR)
    set(VERSION_MINOR 0)
endif()
if(NOT DEFINED VERSION_MICRO)
    set(VERSION_MICRO 0)
endif()
if(NOT DEFINED UKUI_QUICK_VERSION)
    set(UKUI_QUICK_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Qml REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Qml REQUIRED)


if (QT_VERSION_MAJOR EQUAL "5")
    find_package(KF5Wayland)
elseif (QT_VERSION_MAJOR EQUAL "6")
    find_package(KWayland)
endif()

# Find required packages
find_package(PkgConfig REQUIRED)
set(PC_PKGS egl wayland-client gl)
foreach(external_lib IN ITEMS ${PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND TEST_EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()

# Find KF5 modules
find_package(KF5Wayland)

if(NOT TARGET ukui-quick::platform)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../platform
            ${CMAKE_CURRENT_BINARY_DIR}/platform EXCLUDE_FROM_ALL)
endif()

set(WINDOW_THUMBNAIL_SRCS
        ../window-thumbnail-item.cpp
        ../window-thumbnail-item.h
        ../thumbnail-item.cpp
        ../thumbnail-item.h
        ../wlcom-thumbnail.cpp
        ../wlcom-thumbnail.h
        ../thumbnail.h
        ../thumbnail-buffer.h
        ../graphics-memory-buffer.cpp
        ../graphics-memory-buffer.h
        ../workspace-thumbnail-item.cpp
        ../workspace-thumbnail-item.h
        ../discard-texture-resources-runnable.h
)

set(QRC_FILES test.qrc)

add_executable(${PROJECT_NAME} main.cpp ${QRC_FILES} ${WINDOW_THUMBNAIL_SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ../../../../platform
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt::GuiPrivate
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        ${TEST_EXTERNAL_LIBS}
        ukui-quick::platform
)

if (QT_VERSION_MAJOR EQUAL "5")
    target_link_libraries(${PROJECT_NAME} PRIVATE KF5::WaylandClient)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Plasma::KWaylandClient)
endif()