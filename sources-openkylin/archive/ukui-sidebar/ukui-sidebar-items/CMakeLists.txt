cmake_minimum_required(VERSION 3.16)
project(ukui-sidebar-items)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Gui Qml Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Qml Quick REQUIRED)

# see: https://docs.opencv.org/4.x/db/df5/tutorial_linux_gcc_cmake.html
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
set(PLUGIN_EXTERNAL_LIBS "")
if (QT_VERSION_MAJOR EQUAL "5")
    set(PLUGIN_PC_PKGS gsettings-qt)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(PLUGIN_PC_PKGS gsettings-qt6)
endif()
foreach(PC_LIB IN ITEMS ${PLUGIN_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND PLUGIN_EXTERNAL_LIBS ${${PC_LIB}_LIBRARIES})
    endif()
endforeach()

set(PLUGIN_IMPORT_URI "org.ukui.sidebar.items")
set(PLUGIN_INSTALL_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/qml/org/ukui/sidebar/items")

include_directories(.)
include_directories(items)

set(PLUGIN_SRCS
        theme-palette.h theme-palette.cpp
        sidebar-public-plugin.h sidebar-public-plugin.cpp
        items/commons.h
        items/theme-icon.h items/theme-icon.cpp
        items/pixmap-provider.h items/pixmap-provider.cpp
        items/desktop-background.h items/desktop-background.cpp
        items/image-item.cpp items/image-item.h
        )

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS})
target_compile_definitions(${PROJECT_NAME} PRIVATE PLUGIN_IMPORT_URI="${PLUGIN_IMPORT_URI}")
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        ${OpenCV_LIBS}
        ${PLUGIN_EXTERNAL_LIBS}
        )

install(DIRECTORY "qml" DESTINATION "${PLUGIN_INSTALL_PATH}")
install(FILES "qmldir" DESTINATION "${PLUGIN_INSTALL_PATH}")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION "${PLUGIN_INSTALL_PATH}")
