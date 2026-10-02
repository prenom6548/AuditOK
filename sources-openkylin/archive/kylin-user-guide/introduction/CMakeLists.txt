cmake_minimum_required(VERSION 3.5)

project(ukui-new-function-introduction LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(pkgbindir 	/usr/bin/)
set(pkgdesktop  /etc/xdg/autostart/)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Multimedia MultimediaWidgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5 COMPONENTS Core Quick  REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5WindowSystem ${KF5_MINIMUM_VERSION} REQUIRED)
find_package(X11 REQUIRED)
find_package(Qt5 ${QT_MINIMUM_VERSION}  REQUIRED   X11Extras)
find_package(Qt5X11Extras ${REQUIRED_QT_VERSION} REQUIRED)

#sdk
pkg_check_modules(KYSDKWAYLANDHELPER_PKG kysdk-waylandhelper)
pkg_check_modules(KDKINFO REQUIRED kysdk-sysinfo)
pkg_check_modules(QGS REQUIRED gsettings-qt)

if(ANDROID)
    add_library(ukui-new-function-introduction SHARED
      main.cpp
      qml.qrc
      xatom-helper.h
      xatom-helper.cpp
      processmanager.h
      processmanager.cpp
      qtsingleapplication.h
      qtsingleapplication.cpp
      qtlocalpeer.h
      qtlocalpeer.cpp
      qtlockedfile.h
      qtlockedfile.cpp
    )
else()
    add_executable(ukui-new-function-introduction
      main.cpp
      qml.qrc
      xatom-helper.h
      xatom-helper.cpp
      processmanager.h
      processmanager.cpp
      qtsingleapplication.h
      qtsingleapplication.cpp
      qtlocalpeer.h
      qtlocalpeer.cpp
      qtlockedfile.h
      qtlockedfile.cpp
      qtlockedfile_unix.cpp
    )
endif()

pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)
include_directories(${GLIB2_INCLUDE_DIRS} ${gsettings-qt_INCLUDE_DIRS})
link_libraries(glib-2.0.so gio-2.0.so)

target_compile_definitions(ukui-new-function-introduction
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(ukui-new-function-introduction
  PRIVATE Qt5::Core Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets Qt5::Quick Qt5::DBus  KF5::WindowSystem Qt5::X11Extras -lX11 -lgsettings-qt ukui-log4qt ${GLIB2_LIBRARIES} ${KDKINFO_LIBRARIES}  ${KYSDKWAYLANDHELPER_PKG_LIBRARIES})
target_include_directories(ukui-new-function-introduction PRIVATE ${KYSDKWAYLANDHELPER_PKG_INCLUDE_DIRS} ${KDKINFO_INCLUDE_DIRS})
target_link_directories(ukui-new-function-introduction PRIVATE ${KYSDKWAYLANDHELPER_PKG_LIBRARY_DIRS} ${KDKINFO_LIBRARY_DIRS})
install(TARGETS ukui-new-function-introduction DESTINATION ${pkgbindir})
install(FILES
    data/ukui-new-function-introduction.desktop
    DESTINATION "/etc/xdg/autostart/"
    COMPONENT Runtime)
