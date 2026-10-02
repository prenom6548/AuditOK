cmake_minimum_required(VERSION 3.16)

# 调试多任务视图需要取消注释下面
add_definitions("-Wall -g")

project(ukui-window-switch-wlcom)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KF5_DEP_VERSION "5.54.0")

find_package(KF5Config REQUIRED)
find_package(KF5GlobalAccel REQUIRED)
find_package(KF5WindowSystem)
find_package(KF5I18n REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Concurrent Quick Qml LinguistTools DBus REQUIRED)
find_package(ukui-search REQUIRED)
find_package(Qt5Xdg REQUIRED)
find_package(ukui-quick COMPONENTS platform core)

pkg_check_modules(KDK REQUIRED kysdk-sysinfo)
pkg_check_modules(DATACOLLECT REQUIRED kysdk-datacollect)
pkg_check_modules(GL REQUIRED gl)

include_directories(${KDK_INCLUDE_DIRS})
include_directories(${DATACOLLECT_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GL})


set(WLCSRCS main.cpp
        log-utils.h log-utils.cpp
        ../desktopbackground.cpp
        multitaskviewmanagerbywlcom.cpp
        dbusservice.cpp
        ../windowmanagerinterface.cpp
        ../desktoplistmodel.h
        ../desktoplistmodel.cpp
        ../appwindowlistmodel.h
        ../appwindowlistmodel.cpp
        ../appwindowfiltermodel.h
        ../appwindowfiltermodel.cpp
        ../ukui-flow.cpp
        ../ukui-flow.h
        ../screens-manager.cpp
        ../screens-manager.h
        ../globalsettings.h
        ../globalsettings.cpp
        ../abstractdesktopmanager.h
        ../desktopmanager.h
        ../desktopmanager.cpp
        kywcdesktopmanager.h
        kywcdesktopmanager.cpp
        ../windowsmanager.h
        ../windowsmanager.cpp
)

add_subdirectory(qtsingleapplication)
include_directories(qtsingleapplication/src)

qt5_add_resources(WLCSRCS ../data.qrc)

add_executable(ukui-window-switch-wlcom
    ${WLCSRCS}
)
target_include_directories(ukui-window-switch-wlcom PRIVATE ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
target_link_directories(ukui-window-switch-wlcom PRIVATE KF5::WindowSystem)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DISABLE_LOG_FILE>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:QT_QML_DEBUG>)

target_link_libraries(
        ukui-window-switch-wlcom
        PRIVATE
        Qt5::Core
        Qt5::GuiPrivate
        Qt5::Quick
        Qt5::QuickPrivate
        KF5::WindowSystem
        KF5::GlobalAccel
        Qt5::Concurrent
        Qt5::DBus
        Qt5::Qml
        Qt::QmlPrivate
        qtsingleapplication
        ${QGSettings_LIBRARIES}
        ${KDK_LINK_LIBRARIES}
        ${DATACOLLECT_LINK_LIBRARIES}
        ${OPENGL_gl_LIBRARY}
        ${EGL_LIBRARY}
        ${GL_LIBRARIES}
        ukui-search
        Qt5Xdg
        ukui-quick::platform
        ukui-quick::core
)

install(TARGETS ukui-window-switch-wlcom DESTINATION /usr/bin)
