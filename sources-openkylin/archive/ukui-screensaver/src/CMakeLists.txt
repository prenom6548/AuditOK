
find_package(Qt5 COMPONENTS Core Widgets DBus X11Extras Xml Network Svg Multimedia MultimediaWidgets Sql)

find_package(PkgConfig REQUIRED)
find_package(KF5Wayland CONFIG)
find_package(Qt5WebKitWidgets REQUIRED)

pkg_check_modules(GIOUNIX2 REQUIRED gio-unix-2.0)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)

find_package(X11 REQUIRED)
pkg_check_modules(KYSDKSYSTIME REQUIRED kysdk-systime)
pkg_check_modules(KYSDKWINDOWHELPER REQUIRED kysdk-ukuiwindowhelper)
if (KYSDK_UKUIWINDOWHELPER_FOUND)
    include_directories(${KYSDK_UKUIWINDOWHELPER_INCLUDE_DIRS})
endif()
pkg_check_modules(KYSDKQTWIDGETS kysdk-qtwidgets)
pkg_check_modules(XTST REQUIRED xtst)
pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(QGS REQUIRED gsettings-qt)
pkg_check_modules(MMIX REQUIRED libmatemixer)
pkg_check_modules(kylin-nm-base REQUIRED kylin-nm-base)
pkg_check_modules(LIGHTDM-QT5-3 REQUIRED liblightdm-qt5-3)
pkg_check_modules(LIBSYSTEMD REQUIRED libsystemd)
pkg_check_modules(LIBXRANDR REQUIRED xrandr)

# 查找pam动态库全路径并缓存到PAM_LIBRARIES变量
find_library(PAM_LIBRARIES pam)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/common)
include_directories(${PROJECT_SOURCE_DIR}/src/widgets)
include_directories(${PROJECT_SOURCE_DIR}/src/QtSingleApplication)
include_directories(${PROJECT_SOURCE_DIR}/src/dbusifs)
include_directories(${PROJECT_SOURCE_DIR}/VirtualKeyboard/src)
include_directories(${PROJECT_SOURCE_DIR}/screensaver)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/common COMMON)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/widgets WIDGEGS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lock-dialog LOCK_DIALOG)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lock-backend LOCK_BACKEND)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lock-command LOCK_COMMAND)

add_definitions(-DRESOURCE_PATH="/usr/share/ukui-screensaver/")
add_definitions(-DENABLEPQ)

set(SINGLE_GUI_APPLICATION_SRCS
    ${PROJECT_SOURCE_DIR}/src/QtSingleApplication/qtsingleapplication.cpp
    ${PROJECT_SOURCE_DIR}/src/QtSingleApplication/qtlocalpeer.cpp
    )
set(SINGLE_CORE_APPLICATION_SRCS
    ${PROJECT_SOURCE_DIR}/src/QtSingleApplication/qtsinglecoreapplication.cpp
    ${PROJECT_SOURCE_DIR}/src/QtSingleApplication/qtlocalpeer.cpp
    )

include_directories(
    ${X11_INCLUDE_DIRS}
    ${XTST_INCLUDE_DIRS}
    ${XCB_INCLUDE_DIRS}
    ${QGS_INCLUDE_DIRS}
    ${GIOUNIX2_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${MMIX_INCLUDE_DIRS}
    ${kylin-nm-base_INCLUDE_DIRS}
    ${LIGHTDM-QT5-3_INCLUDE_DIRS}
    ${LIBSYSTEMD_INCLUDE_DIRS}
    ${LIBXRANDR_INCLUDE_DIRS}
    )

set(EXTRA_LIBS
    ${EXTRA_LIBS}
    ${X11_LIBRARIES}
    ${XTST_LIBRARIES}
    ${XCB_LIBRARIES}
    ${QGS_LIBRARIES}
    ${GIOUNIX2_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${MMIX_LIBRARIES}
    ${LIBXRANDR_LIBRARIES}
    -lrt
    -lpthread
    )

option(SUPPORT_X11_BACKGROUND "Support draw x11 background" OFF)
if (SUPPORT_X11_BACKGROUND)
    add_definitions(-DSUPPORT_X11_BACKGROUND)
endif()

option(SUPPORT_NET_ICON "Support show network icon from library" ON)
if (SUPPORT_NET_ICON)
    set(EXTRA_LIBS
        ${EXTRA_LIBS}
        -llibnm-icon-kylin
        )
    add_definitions(-DSUPPORT_NET_ICON)
endif()

option(SUPPORT_TABLET_MODE "Support tablet mode check" OFF)
if (SUPPORT_TABLET_MODE)
    set(BACKEND_EXTRA_LIBS
        ${BACKEND_EXTRA_LIBS}
        -lukuiinputgatherclient
        )
    add_definitions(-DSUPPORT_TABLET_MODE)
endif()

#qt5_wrap_ui(dialog_SRC
#    lockwidget.ui
#    surewindow.ui
#    )

qt5_add_resources(dialog_SRC
	screensaver/default.qrc
        assets.qrc
    )

qt5_wrap_cpp(dialog_SRC
    userinfo.h
    agreementinfo.h
    dbusifs/freedesktophelper.h
    dbusifs/greeterconfig.h
    )

set(dialog_SRC
    ${dialog_SRC}
    ${COMMON}
    ${WIDGEGS}
    ${SINGLE_GUI_APPLICATION_SRCS}
    ${LOCK_DIALOG}
    userinfo.cpp
    agreementinfo.cpp
    dbusifs/uniauthservice.cpp
    dbusifs/biometrichelper.cpp
    dbusifs/freedesktophelper.cpp
    dbusifs/greeterconfig.cpp
    dbusifs/giodbus.cpp
    common/configuration.cpp
    common/displayservice.cpp
    common/capslockstatehelper.cpp
    )

add_executable(ukui-screensaver-dialog ${dialog_SRC})
add_definitions(-DAPP_API_MAJOR=0
                -DAPP_API_MINOR=11
                -DAPP_API_FUNC=0
                )
target_include_directories(ukui-screensaver-dialog PRIVATE ${KYSDKSYSTIME_INCLUDE_DIRS} ${KYSDKWINDOWHELPER_INCLUDE_DIRS} ${KYSDKQTWIDGETS_INCLUDE_DIRS})
target_link_directories(ukui-screensaver-dialog PRIVATE ${KYSDKSYSTIME_LIBRARY_DIRS} ${KYSDKWINDOWHELPER_LIBRARY_DIRS} ${KYSDKQTWIDGETS_LIBRARY_DIRS})
target_link_libraries(ukui-screensaver-dialog
	Qt5::Core
	Qt5::Widgets
	Qt5::DBus
	Qt5::Svg
	Qt5::X11Extras
        Qt5::Network
        Qt5::Widgets
        KF5::WaylandClient
        ${KYSDKSYSTIME_LIBRARIES}
        ${KYSDKWINDOWHELPER_LIBRARIES}
        ${KYSDKQTWIDGETS_LIBRARIES}
	${EXTRA_LIBS}
        VirtualKeyboard
        ukui-log4qt
        -ldl
	-lcrypto
        -lqrencode
        pwquality
        )

qt5_add_dbus_adaptor(backend_SRC
    lock-backend/org.ukui.ScreenSaver.xml
    lock-backend/dbusupperinterface.h
    DbusUpperInterface
    )

qt5_wrap_cpp(backend_SRC
       userinfo.h
       agreementinfo.h
       dbusifs/freedesktophelper.h
       dbusifs/greeterconfig.h
       dbusifs/upowerdbus.h
       dbusifs/upowerbatterydbus.h
       dbusifs/upowerlinepowerdbus.h
       dbusifs/upm_upowerdbus.h
       dbusifs/usdblockshortcut.h
       ukccplugins/sessiondbus/screensaverinterface.h
       ukccplugins/sessiondbus/screenlockinterface.h
    )

set(backend_SRC
    ${backend_SRC}
    ${LOCK_BACKEND}
    ${SINGLE_CORE_APPLICATION_SRCS}
    dbusifs/login1helper.cpp
    dbusifs/usdhelper.cpp
    dbusifs/upowerhelper.cpp
    dbusifs/device.cpp
    dbusifs/accountshelper.cpp
    dbusifs/sessionhelper.cpp
    dbusifs/uniauthservice.cpp
    dbusifs/biometrichelper.cpp
    dbusifs/freedesktophelper.cpp
    dbusifs/giodbus.cpp
    dbusifs/kglobalaccelhelper.cpp
    dbusifs/machinemodel.cpp
    dbusifs/libinputswitchevent.cpp
    dbusifs/systemupgradehelper.cpp
    dbusifs/dbusservermanager.cpp
    dbusifs/networkmanagerhelper.cpp
    dbusifs/filemanagerhelper.cpp
    dbusifs/greeterconfig.cpp
    dbusifs/upowerdbus.cpp
    dbusifs/upowerbatterydbus.cpp
    dbusifs/upowerlinepowerdbus.cpp
    dbusifs/upm_upowerdbus.cpp
    dbusifs/usdblockshortcut.cpp
    userinfo.cpp
    agreementinfo.cpp
    common/global_utils.cpp
    common/configuration.cpp
    common/biodefines.cpp
    common/kyrsac.cpp
    common/xdgdesktopfile.cpp
    common/xdgdirs.cpp
    ukccplugins/sessiondbus/screensaverinterface.cpp
    ukccplugins/sessiondbus/screenlockinterface.cpp
    )

add_executable(ukui-screensaver-backend ${backend_SRC})
target_link_libraries(ukui-screensaver-backend
    Qt5::Core
    Qt5::DBus
    Qt5::Network
    ${PAM_LIBRARIES}
    ${LIGHTDM-QT5-3_LIBRARIES}
    ${QGS_LIBRARIES}
    ${LIBSYSTEMD_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ukui-log4qt
    -ldl
    -lcrypto
    ${BACKEND_EXTRA_LIBS}
    )

set(command_SRC
    ${LOCK_COMMAND}
    )
add_executable(ukui-screensaver-command ${command_SRC})
target_link_libraries(ukui-screensaver-command Qt5::Core Qt5::DBus ukui-log4qt)


add_subdirectory(VirtualKeyboard)
add_subdirectory(screensaver)
add_subdirectory(ukccplugins)

add_dependencies(ukui-screensaver-dialog VirtualKeyboard ukui-screensaver-default screensaver-default Screensaver)

install(TARGETS
    ukui-screensaver-dialog
    ukui-screensaver-backend
    ukui-screensaver-command
    DESTINATION bin)

install(DIRECTORY assets/badges DESTINATION /usr/share/ukui-screensaver/images/)
