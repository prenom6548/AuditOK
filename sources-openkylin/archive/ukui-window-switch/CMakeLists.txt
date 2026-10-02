cmake_minimum_required(VERSION 3.16)

project(ukui-window-switch)

find_package(PkgConfig)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)
include_directories(${QGSettings_INCLUDE_DIRS})
find_package(Qt5 CONFIG REQUIRED COMPONENTS Core Gui)
find_package(KF5CoreAddons REQUIRED)
find_package(KF5WindowSystem REQUIRED)

get_target_property(QT_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE} IMPORTED_LOCATION)
macro(query_qmake args output)
    execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query ${args} OUTPUT_VARIABLE ${output} RESULT_VARIABLE test1 RESULT_VARIABLE result OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT ${result} EQUAL 0)
        message(FATAL_ERROR "exec ${QT_QMAKE_EXECUTABLE} failed, with args: ${args}, error message: ${output}")
    endif()
endmacro()

query_qmake("QT_INSTALL_PLUGINS" QT_INSTALL_PLUGINS)

add_subdirectory(windowsview)
add_subdirectory(windowswitchers)

if(BUILD_TEST)
    add_subdirectory(tests)
endif()
