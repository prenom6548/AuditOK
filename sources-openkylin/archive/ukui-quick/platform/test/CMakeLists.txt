cmake_minimum_required(VERSION 3.14)
project(platform-test)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Qml Widgets QuickControls2 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Qml Widgets QuickControls2 REQUIRED)

set(QRC_FILES qml.qrc)
set(TEST_QML_IMPORT_ROOT ${CMAKE_CURRENT_BINARY_DIR}/qml-imports)
set(PROJECT_SOURCES
        main.cpp
        output-manager-monitor.cpp
        output-manager-monitor.h
        workspace-demo-controller.cpp
        workspace-demo-controller.h
        ${QRC_FILES}
)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::QuickControls2
        ukui-quick::platform
)

add_dependencies(${PROJECT_NAME} ukui-quick-items-plugin ukui-quick-platform-plugin)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        TEST_QML_IMPORT_ROOT="${TEST_QML_IMPORT_ROOT}"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
                ${TEST_QML_IMPORT_ROOT}/org/ukui/quick/platform
                ${TEST_QML_IMPORT_ROOT}/org/ukui/quick/items
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/../qmldir
                ${TEST_QML_IMPORT_ROOT}/org/ukui/quick/platform/qmldir
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:ukui-quick-platform-plugin>
                ${TEST_QML_IMPORT_ROOT}/org/ukui/quick/platform/$<TARGET_FILE_NAME:ukui-quick-platform-plugin>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/../../items/qmldir
                ${TEST_QML_IMPORT_ROOT}/org/ukui/quick/items/qmldir
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:ukui-quick-items-plugin>
                ${TEST_QML_IMPORT_ROOT}/org/ukui/quick/items/$<TARGET_FILE_NAME:ukui-quick-items-plugin>
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/../../items/qml
                ${TEST_QML_IMPORT_ROOT}/org/ukui/quick/items/qml
)

# Capture test executable
find_package(PkgConfig REQUIRED)
find_package(KF5Wayland)
set(CAPTURE_PC_PKGS egl wayland-client gl)
foreach(external_lib IN ITEMS ${CAPTURE_PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND CAPTURE_EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()

set(CAPTURE_SOURCES
        main_capture.cpp
        kywc-capture-thumbnail-item.h
        kywc-capture-thumbnail-item.cpp
        ${QRC_FILES}
)

add_executable(capture-test ${CAPTURE_SOURCES})

# Add wayland protocols include directories
include_directories(${WAYLAND_PROTOCOLS_INCLUDE_DIRS})

target_link_libraries(capture-test
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Quick
        Qt::GuiPrivate
        Qt${QT_VERSION_MAJOR}::DBus
        ${CAPTURE_EXTERNAL_LIBS}
        ukui-quick::platform
        KF5::WaylandClient
        Qt5::Gui_EGL

)
