cmake_minimum_required(VERSION 3.14)
project(ukui-file-metadata LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)
include(GNUInstallDirs)
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CheckStructHasMember)

#Define QT_MAJOR_VERSION
if (DEFINED QT_MAJOR_VERSION)
    return()
endif()

if (TARGET Qt5::Core)
    set(QT_MAJOR_VERSION 5)
elseif (TARGET Qt6::Core)
    set(QT_MAJOR_VERSION 6)
else()
    option(BUILD_WITH_QT6 "Build against Qt 6" OFF)

    if (BUILD_WITH_QT6)
        set(QT_MAJOR_VERSION 6)
    else()
        set(QT_MAJOR_VERSION 5)
    endif()
endif()

# Dependencies
set(REQUIRED_QT_VERSION 5.12.8)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)


set(POPPLER_LIB "")
if(QT_VERSION_MAJOR EQUAL 5)
    set(POPPLER_LIB poppler-qt5)
elseif (QT_VERSION_MAJOR EQUAL 6)
    set(POPPLER_LIB poppler-qt6)
endif ()

find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED)

set(UKUI_FILE_METADATA_PC_PKGS uchardet libavcodec libavformat libavutil libswscale taglib minizip tesseract lept)
foreach(PC_LIB IN LISTS UKUI_FILE_METADATA_PC_PKGS)
    string(TOUPPER "${PC_LIB}" PC_PREFIX)
    # Normalize libav* package names to the shorter imported-target prefixes
    # used elsewhere in the tree, e.g. libavcodec -> AVCODEC.
    string(REGEX REPLACE "^LIB" "" PC_PREFIX "${PC_PREFIX}")
    pkg_check_modules(${PC_PREFIX} REQUIRED IMPORTED_TARGET ${PC_LIB})
endforeach()
# poppler-qt5 / poppler-qt6 do not fit the generic prefix normalization above,
# so keep a stable POPPLER imported-target prefix for consumers.
pkg_check_modules(POPPLER REQUIRED IMPORTED_TARGET ${POPPLER_LIB})

enable_testing()
add_subdirectory(src)
if (BUILD_TESTING)
add_subdirectory(autotests)
add_subdirectory(tests)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
