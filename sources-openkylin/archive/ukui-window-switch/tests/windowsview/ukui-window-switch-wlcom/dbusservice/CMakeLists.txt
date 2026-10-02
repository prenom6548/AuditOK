cmake_minimum_required(VERSION 3.16)
project(test_dbusservice)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -lgcov")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_compile_definitions(TRANSLATION_DIR="/usr/share/ukui-window-switch/translations/")

find_package(ECM 5.54.0 NO_MODULE)
find_package(PkgConfig REQUIRED)
find_package(OpenGL)
find_package(KF5Config REQUIRED)
find_package(KF5GlobalAccel REQUIRED)
find_package(KF5WindowSystem)
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Qml Test Concurrent DBus REQUIRED)
find_package(ukui-quick COMPONENTS platform core REQUIRED)

pkg_check_modules(QGSettings REQUIRED gsettings-qt)
pkg_check_modules(KDK REQUIRED kysdk-sysinfo)
pkg_check_modules(DATACOLLECT REQUIRED kysdk-datacollect)
pkg_check_modules(GL REQUIRED gl)
pkg_check_modules(KYWC REQUIRED kywc)
pkg_check_modules(QT5XDG REQUIRED Qt5Xdg)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../windowsview/ukui-window-switch-wlcom)
set(WINDOWSVIEW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../windowsview)
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    ${SOURCE_DIR}
    ${WINDOWSVIEW_DIR}
    ${TEST_DIR}
    ${QGSettings_INCLUDE_DIRS}
    ${KDK_INCLUDE_DIRS}
    ${DATACOLLECT_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${GL_INCLUDE_DIRS}
    ${KYWC_INCLUDE_DIRS}
    ${QT5XDG_INCLUDE_DIRS}
)

set(TEST_SOURCES
    main.cpp
    test_dbusservice.cpp
    test_dbusservice.h
    mock_multitaskviewmanagerbywlcom.h
    minimal_multitaskviewmanager.h
    ${SOURCE_DIR}/dbusservice.cpp
    ${SOURCE_DIR}/dbusservice.h
    ${SOURCE_DIR}/qtsingleapplication/src/qtsingleapplication.cpp
    ${SOURCE_DIR}/qtsingleapplication/src/qtsingleapplication.h
    ${SOURCE_DIR}/qtsingleapplication/src/qtlocalpeer.cpp
    ${SOURCE_DIR}/qtsingleapplication/src/qtlocalpeer.h
)

add_executable(test_dbusservice ${TEST_SOURCES})

target_link_libraries(test_dbusservice
    Qt5::Core
    Qt5::Widgets
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
    ${QGSettings_LIBRARIES}
    ${KDK_LINK_LIBRARIES}
    ${DATACOLLECT_LINK_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${EGL_LIBRARY}
    ${GL_LIBRARIES}
    ${QT5XDG_LIBRARIES}
    ${KYWC_LIBRARIES}
    ukui-search
    Qt5Xdg
    ukui-quick::platform
    ukui-quick::core
    kywc
    EGL
    pthread
)

target_compile_definitions(test_dbusservice PRIVATE QT_QML_DEBUG)
