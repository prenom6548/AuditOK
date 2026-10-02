cmake_minimum_required(VERSION 3.1.0)

project(kylin-device-daemon)

#判断编译器类型,如果是gcc编译器,则在编译选项中加入c++11支持
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-g -std=c++11 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++11")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets Network LinguistTools REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(Qt5X11Extras ${REQUIRED_QT_VERSION} REQUIRED)

pkg_check_modules(KYSDKWAYLANDHELPER_PKG kysdk-waylandhelper)
pkg_check_modules(KYSDKSOUNDEFFECTS_PKG kysdk-soundeffects)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0 udisks2 x11)
pkg_check_modules(QGS REQUIRED gsettings-qt)
pkg_check_modules(KYSDKQTWIDGETS_PKG kysdk-qtwidgets)
pkg_check_modules(KDKINFO REQUIRED kysdk-sysinfo)

if(EXISTS "/usr/include/kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h")
    message(STATUS "kywaylandhelper find")
    add_definitions(-DKY_SDK_WAYLANDHELPER)
    pkg_check_modules(KYSDKUKUIWINDOWHELPER_PKG kysdk-ukuiwindowhelper)
    include_directories("/usr/include/kysdk/applications")
endif()

FILE(GLOB TS_FILES ${CMAKE_SOURCE_DIR}/translation/*.ts)
FILE(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/kylin-device-daemon.qrc
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/mainwindow.cpp
        ${CMAKE_SOURCE_DIR}/src/qclickwidget.cpp
        ${CMAKE_SOURCE_DIR}/src/qclickwidget.h
        ${CMAKE_SOURCE_DIR}/src/UnionVariable.cpp
        ${CMAKE_SOURCE_DIR}/src/UnionVariable.h
        ${CMAKE_SOURCE_DIR}/src/ejectInterface.cpp
        ${CMAKE_SOURCE_DIR}/src/ejectInterface.h
        ${CMAKE_SOURCE_DIR}/src/clickLabel.h
        ${CMAKE_SOURCE_DIR}/src/device-operation.h
        ${CMAKE_SOURCE_DIR}/src/device-operation.cpp
        ${CMAKE_SOURCE_DIR}/src/clickLabel.cpp
        ${CMAKE_SOURCE_DIR}/src/device-manager.h
        ${CMAKE_SOURCE_DIR}/src/device-manager.cpp
        ${CMAKE_SOURCE_DIR}/src/repair-dialog-box.h
        ${CMAKE_SOURCE_DIR}/src/repair-dialog-box.cpp
        ${CMAKE_SOURCE_DIR}/src/MainController.h
        ${CMAKE_SOURCE_DIR}/src/MainController.cpp
        ${CMAKE_SOURCE_DIR}/src/MacroFile.h
        ${CMAKE_SOURCE_DIR}/src/interactivedialog.cpp
        ${CMAKE_SOURCE_DIR}/src/interactivedialog.h
        ${CMAKE_SOURCE_DIR}/src/gpartedinterface.cpp
        ${CMAKE_SOURCE_DIR}/src/gpartedinterface.h
        ${CMAKE_SOURCE_DIR}/src/mainwindow.ui
        ${CMAKE_SOURCE_DIR}/src/mainwindow.h
        ${CMAKE_SOURCE_DIR}/src/fdapplication.cpp
        ${CMAKE_SOURCE_DIR}/src/fdapplication.h
        ${CMAKE_SOURCE_DIR}/src/fdframe.cpp
        ${CMAKE_SOURCE_DIR}/src/fdframe.h
        ${CMAKE_SOURCE_DIR}/src/flashdiskdata.cpp
        ${CMAKE_SOURCE_DIR}/src/flashdiskdata.h
        ${CMAKE_SOURCE_DIR}/src/fdclickwidget.cpp
        ${CMAKE_SOURCE_DIR}/src/fdclickwidget.h
        ${CMAKE_SOURCE_DIR}/3rd-parties/QtSingleApplication/qtsingleapplication.h
        ${CMAKE_SOURCE_DIR}/3rd-parties/QtSingleApplication/qtsingleapplication.cpp
        ${CMAKE_SOURCE_DIR}/3rd-parties/QtSingleApplication/qtlocalpeer.h
        ${CMAKE_SOURCE_DIR}/3rd-parties/QtSingleApplication/qtlocalpeer.cpp
        ${CMAKE_SOURCE_DIR}/src/datacdrom.h
        ${CMAKE_SOURCE_DIR}/src/datacdrom.cpp
        ${CMAKE_SOURCE_DIR}/src/basewidget.h
        ${CMAKE_SOURCE_DIR}/src/basewidget.cpp
        ${CMAKE_SOURCE_DIR}/src/config/xatom-helper.h
        ${CMAKE_SOURCE_DIR}/src/config/xatom-helper.cpp)

QT5_CREATE_TRANSLATION(QM_FILES ${PROJECT_SOURCE_DIR} ${TS_FILES})

include_directories(${CMAKE_SOURCE_DIR}/3rd-parties)
include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${QGS_INCLUDE_DIRS})
include_directories(${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS})

add_executable(kylin-device-daemon ${QM_FILES} ${SRC_FILES})

add_executable(test-repair-dialog
    ${CMAKE_SOURCE_DIR}/src/device-manager.h
    ${CMAKE_SOURCE_DIR}/src/device-manager.cpp
    ${CMAKE_SOURCE_DIR}/src/repair-dialog-box.h
    ${CMAKE_SOURCE_DIR}/src/repair-dialog-box.cpp
    ${CMAKE_SOURCE_DIR}/src/device-operation.h
    ${CMAKE_SOURCE_DIR}/src/device-operation.cpp
    ${CMAKE_SOURCE_DIR}/src/test-repair-dialog.cpp
)
target_link_libraries(test-repair-dialog Qt5::Widgets Qt5::DBus ${GLIB2_LIBRARIES} ${QGS_LIBRARIES} KF5::WindowSystem)

add_definitions(-DQT_NO_KEYWORDS)
add_definitions(-DQT_MESSAGELOGCONTEXT)

target_link_libraries(${PROJECT_NAME}  Qt5::Widgets Qt5::DBus Qt5::Network Qt5::X11Extras ${GLIB2_LIBRARIES} ${QGS_LIBRARIES} KF5::WindowSystem -lukui-log4qt X11 -lkysdk-qtwidgets)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDKWAYLANDHELPER_PKG_INCLUDE_DIRS} ${KYSDKSOUNDEFFECTS_PKG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDKWAYLANDHELPER_PKG_LIBRARY_DIRS} ${KYSDKSOUNDEFFECTS_PKG_LIBRARY_DIRS} KF5::WindowSystem ${KDKINFO_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} ${KYSDKWAYLANDHELPER_PKG_LIBRARIES} ${KYSDKSOUNDEFFECTS_PKG_LIBRARIES} ${KDKINFO_LIBRARIES})

if(EXISTS "/usr/include/kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h")
    target_link_libraries(${PROJECT_NAME} -lkysdk-ukuiwindowhelper)
    target_link_libraries(test-repair-dialog -lkysdk-ukuiwindowhelper)
    
    if(KYSDKUKUIWINDOWHELPER_PKG_FOUND)
        target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDKUKUIWINDOWHELPER_PKG_INCLUDE_DIRS})
        target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDKUKUIWINDOWHELPER_PKG_LIBRARY_DIRS})
        target_link_libraries(${PROJECT_NAME} ${KYSDKUKUIWINDOWHELPER_PKG_LIBRARIES})
    endif()
endif()

install(TARGETS kylin-device-daemon DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/data/kylin-device-daemon.lock DESTINATION "/usr/share/ukui/ukui-panel" COMPONENT Runtime)
install(FILES ${CMAKE_SOURCE_DIR}/data/kylin-device-daemon.ini DESTINATION "/etc/kylin-device-daemon/" COMPONENT Runtime)
install(FILES ${CMAKE_SOURCE_DIR}/data/kylin-device-daemon.desktop DESTINATION "/etc/xdg/autostart/" COMPONENT Runtime)
install(FILES ${QM_FILES} DESTINATION "/usr/share/kylin-device-daemon/" COMPONENT Runtime)
install(FILES ${CMAKE_SOURCE_DIR}/data/org.kylin-device-daemon.autoload.gschema.xml DESTINATION "/usr/share/glib-2.0/schemas" COMPONENT Runtime)
