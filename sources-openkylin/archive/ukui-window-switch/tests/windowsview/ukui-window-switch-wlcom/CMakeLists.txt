cmake_minimum_required(VERSION 3.16)

project(test_main)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC --coverage")

find_package(ECM 5.54.0 NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(OpenGL)
find_package(KF5Config REQUIRED)
find_package(KF5GlobalAccel REQUIRED)
find_package(KF5WindowSystem)
find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Qml Test Concurrent DBus REQUIRED)
find_package(ukui-quick COMPONENTS platform core REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)
pkg_check_modules(QT5XDG REQUIRED Qt5Xdg)

pkg_check_modules(KDK REQUIRED kysdk-sysinfo)
pkg_check_modules(DATACOLLECT REQUIRED kysdk-datacollect)
pkg_check_modules(GL REQUIRED gl)

# Use relative paths from project root
set(SOURCE_ROOT ${CMAKE_SOURCE_DIR}/../../..)

include_directories(${SOURCE_ROOT}/windowsview/ukui-window-switch-wlcom)
include_directories(${SOURCE_ROOT}/windowsview)
include_directories(${SOURCE_ROOT})
include_directories(${KDK_INCLUDE_DIRS})
include_directories(${DATACOLLECT_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GL})
include_directories(${ukui-quick_INCLUDE_DIRS})
include_directories(${QGSettings_INCLUDE_DIRS})
include_directories(${QT5XDG_INCLUDE_DIRS})
include_directories(/usr/include/qt5xdg)
include_directories(/usr/include/qt5xdgiconloader)
include_directories(${SOURCE_ROOT}/windowsview/ukui-window-switch-wlcom/qtsingleapplication/src)

# Add qtsingleapplication subdirectory
add_subdirectory(${SOURCE_ROOT}/windowsview/ukui-window-switch-wlcom/qtsingleapplication qtsingleapplication_build)

# Only test log-utils.cpp which is relatively independent
set(EXTRA_SOURCES
    ${SOURCE_ROOT}/windowsview/ukui-window-switch-wlcom/log-utils.cpp
)

add_executable(test_main
    test_main.cpp
    ${EXTRA_SOURCES}
)

target_include_directories(test_main PRIVATE ${Qt5Quick_PRIVATE_INCLUDE_DIRS})

# qtsingleapplication is built as part of this test build
# Find the library after build completes
set(QTSINGLEAPPLICATION_LIB ${CMAKE_BINARY_DIR}/qtsingleapplication_build/libqtsingleapplication.a)

target_link_libraries(test_main
    PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::GuiPrivate
    Qt5::Quick
    Qt5::QuickPrivate
    Qt5::Qml
    Qt5::Test
    Qt5::Concurrent
    Qt5::DBus
    KF5::WindowSystem
    KF5::GlobalAccel
    --coverage
    gcov
    ${QTSINGLEAPPLICATION_LIB}
    ${KDK_LINK_LIBRARIES}
    ${DATACOLLECT_LINK_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${EGL_LIBRARY}
    ${GL_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${QT5XDG_LIBRARIES}
    ukui-quick::platform
    ukui-quick::core
)

target_compile_definitions(test_main PRIVATE QT_QML_DEBUG)
