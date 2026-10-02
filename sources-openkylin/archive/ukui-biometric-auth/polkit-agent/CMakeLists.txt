cmake_minimum_required(VERSION 2.6)
project(ukui-polkit-agent)

pkg_check_modules(QGS REQUIRED gsettings-qt)
pkg_check_modules(KDKINFO REQUIRED kysdk-sysinfo)
pkg_check_modules(KDKWIDGETS REQUIRED kysdk-qtwidgets)
pkg_check_modules(KYSDKWAYLANDHELPER REQUIRED kysdk-waylandhelper)
pkg_check_modules(KYSDKWINDOWHELPER REQUIRED kysdk-ukuiwindowhelper)

find_package(PolkitQt5-1 REQUIRED 0.103.0)
find_package(Qt5 COMPONENTS Core Widgets DBus X11Extras Xml Network Svg)
find_package(KF5WindowSystem)
find_package(X11 REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(LIB_ARCH_PATH /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE})

configure_file(
    ${PROJECT_SOURCE_DIR}/data/polkit-ukui-authentication-agent-1.desktop.in
    ${PROJECT_BINARY_DIR}/data/polkit-ukui-authentication-agent-1.desktop
    )

include_directories(
        ${QGS_INCLUDE_DIRS}
        ${KDKINFO_INCLUDE_DIRS}
        ${KDKWIDGETS_INCLUDE_DIRS}
        ${KYSDKWAYLANDHELPER_INCLUDE_DIRS}
        ${KYSDKWINDOWHELPER_INCLUDE_DIRS}
        )

link_directories(
    ${KDKINFO_LIBRARY_DIRS}
    ${KDKWIDGETS_LIBRARY_DIRS}
    ${KYSDKWAYLANDHELPER_LIBRARY_DIRS}
    ${KYSDKWINDOWHELPER_LIBRARY_DIRS}
    )

set(EXTRA_LIBS
        ${EXTRA_LIBS}
        ${QGS_LIBRARIES}
        ${KDKINFO_LIBRARIES}
        ${KDKWIDGETS_LIBRARIES}
        ${KYSDKWAYLANDHELPER_LIBRARIES}
        ${KYSDKWINDOWHELPER_LIBRARIES}
        )

include_directories(
    ${POLKITQT-1_INCLUDE_DIR}
    ../common
    ../bioauth/include
    )

qt5_add_resources(polkit_SRCS assets.qrc)

qt5_wrap_ui(polkit_SRCS
    src/mainwindow.ui
    )

set(polkit_SRCS
    ${polkit_SRCS}
    src/PolkitAgent.cpp
    src/mainwindow.cpp
    src/PolkitListener.cpp
    src/sessionmanager.cpp
    src/users.cpp
    src/pam-tally.c
    src/modeButton.cpp
    src/kalabel.cpp
    ../common/generic.cpp
    src/fullscreenbackground.h
    src/fullscreenbackground.cpp
    src/usdblockshortcut.h
    src/usdblockshortcut.cpp
    )

add_executable(polkit-ukui-authentication-agent-1 ${polkit_SRCS})
target_include_directories(polkit-ukui-authentication-agent-1 PRIVATE ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS})
target_link_directories(polkit-ukui-authentication-agent-1 PRIVATE ${KYSDKQTWIDGETS_PKG_LIBRARY_DIRS})
target_link_libraries(polkit-ukui-authentication-agent-1
    Qt5::Core Qt5::Widgets Qt5::DBus ${EXTRA_LIBS} KF5::WindowSystem
    ${POLKITQT-1_LIBRARIES}
    ${KYSDKQTWIDGETS_PKG_LIBRARIES}
    BioAuthWidgets
    -lrt
    -lukui-log4qt
    )
install(TARGETS polkit-ukui-authentication-agent-1
    DESTINATION lib/${CMAKE_LIBRARY_ARCHITECTURE}/ukui-polkit)

# 翻译文件
file(GLOB ts_files i18n_ts/*.ts)
qt5_add_translation(qm_files ${ts_files})
add_custom_target(polkit_i18n DEPENDS ${qm_files} SOURCES ${ts_files})
add_dependencies(polkit-ukui-authentication-agent-1 polkit_i18n)

install(FILES ${qm_files} DESTINATION ${UKUI_BIOMETRIC_DIR}/i18n_qm/polkit)
install(FILES ${PROJECT_BINARY_DIR}/data/polkit-ukui-authentication-agent-1.desktop
    DESTINATION /etc/xdg/autostart)
install(FILES ${PROJECT_BINARY_DIR}/data/polkit-ukui-authentication-agent-1.desktop
    DESTINATION /usr/share/applications)
