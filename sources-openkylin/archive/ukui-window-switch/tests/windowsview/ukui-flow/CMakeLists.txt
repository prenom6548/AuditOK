cmake_minimum_required(VERSION 3.16)

project(test_ukui_flow)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -coverage -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")

find_package(ECM 5.54.0 NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(OpenGL)
find_package(EGL MODULE)
find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Concurrent Quick Qml LinguistTools DBus Test REQUIRED)

pkg_check_modules(GL REQUIRED gl)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GL})
include_directories(${QGSETTINGS_INCLUDE_DIRS})

# Source files under test
set(TEST_SRCS
    test_ukui-flow.cpp
    test_ukui-flow.h
    ../../../windowsview/ukui-flow.cpp
    ../../../windowsview/ukui-flow.h
)

# Add resources file
qt5_add_resources(TEST_RESOURCES qml.qrc)

# Add executable
add_executable(test_ukui_flow
    ${TEST_SRCS}
    ${TEST_RESOURCES}
)

target_include_directories(test_ukui_flow PRIVATE
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../windowsview
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../windowsview/ukui-window-switch-wlcom
)

target_link_libraries(test_ukui_flow
    Qt5::Core
    Qt5::Gui
    Qt5::GuiPrivate
    Qt5::Quick
    Qt5::QuickPrivate
    Qt5::Qml
    Qt5::Test
    Qt5::Concurrent
    Qt5::DBus
    Qt::QmlPrivate
    ${OPENGL_gl_LIBRARY}
    ${EGL_LIBRARY}
    ${GL_LIBRARIES}
    ${QGSETTINGS_LINK_LIBRARIES}
)
