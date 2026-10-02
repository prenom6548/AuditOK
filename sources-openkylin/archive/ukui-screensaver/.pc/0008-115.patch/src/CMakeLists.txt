find_package(Qt5 COMPONENTS Core Widgets DBus X11Extras Xml Network Svg Multimedia MultimediaWidgets)
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
find_package(KF5Wayland CONFIG)
find_package(KF5WindowSystem)

pkg_check_modules(GIOUNIX2 REQUIRED gio-unix-2.0)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)

find_package(X11 REQUIRED)
pkg_check_modules(KYSDKSYSTIME REQUIRED kysdk-systime)
pkg_check_modules(KYSDKWAYLANDHELPER REQUIRED kysdk-waylandhelper)
pkg_check_modules(XTST REQUIRED xtst)
pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(QGS REQUIRED gsettings-qt)
pkg_check_modules(MMIX REQUIRED libmatemixer)
pkg_check_modules(kylin-nm-base REQUIRED kylin-nm-base)
pkg_check_modules(IMLIB2 REQUIRED imlib2)
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
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lock-checkpasswd LOCK_CHECKPASSWD)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/lock-focushelper LOCK_FOCUSHELPER)

add_definitions(-DRESOURCE_PATH="/usr/share/ukui-screensaver/")

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
    ${IMLIB2_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
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
    ${IMLIB2_LIBRARIES}
    ${LIBXRANDR_LIBRARIES}
    -lrt
    -lpthread
    -llibnm-icon-kylin
    )

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
    dbusifs/giodbus.cpp
    common/configuration.cpp
    common/displayservice.cpp
    )
add_executable(ukui-screensaver-dialog ${dialog_SRC})
add_definitions(-DAPP_API_MAJOR=0
                -DAPP_API_MINOR=11
                -DAPP_API_FUNC=0
                -DSUPPORT_MEDIA_CONTROL
                )
target_include_directories(ukui-screensaver-dialog PRIVATE ${KYSDKSYSTIME_INCLUDE_DIRS} ${KYSDKWAYLANDHELPER_INCLUDE_DIRS})
target_link_directories(ukui-screensaver-dialog PRIVATE ${KYSDKSYSTIME_LIBRARY_DIRS} ${KYSDKWAYLANDHELPER_LIBRARY_DIRS})
target_link_libraries(ukui-screensaver-dialog
	Qt5::Core
	Qt5::Widgets
	Qt5::DBus
	Qt5::Svg
	Qt5::X11Extras
        Qt5::Network
        Qt5::Widgets
        KF5::WaylandClient
        KF5::WindowSystem
        ${KYSDKSYSTIME_LIBRARIES}
        ${KYSDKWAYLANDHELPER_LIBRARIES}
	${EXTRA_LIBS}
        VirtualKeyboard
        Screensaver
        ukui-log4qt
        opencv_imgcodecs
        opencv_imgproc
        opencv_core
        -ldl
	-lcrypto
        )

qt5_add_dbus_adaptor(backend_SRC
    lock-backend/org.ukui.ScreenSaver.xml
    lock-backend/dbusupperinterface.h
    DbusUpperInterface
    )

qt5_wrap_cpp(backend_SRC
       userinfo.h
       agreementinfo.h
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
    dbusifs/enginedevice.cpp
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
    userinfo.cpp
    agreementinfo.cpp
    common/global_utils.cpp
    common/configuration.cpp
    common/biodefines.cpp
    common/rsac.cpp
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
    -lukuiinputgatherclient
    )

set(command_SRC
    ${LOCK_COMMAND}
    )
add_executable(ukui-screensaver-command ${command_SRC})
target_link_libraries(ukui-screensaver-command Qt5::Core Qt5::DBus ukui-log4qt)

set(checkpass_SRC
    ${LOCK_CHECKPASSWD}
    )
add_executable(ukui-screensaver-checkpass ${checkpass_SRC})
target_link_libraries(ukui-screensaver-checkpass ${PAM_LIBRARIES})

set(focushelper_SRCS
    ${focushelper_SRCS}
    ${LOCK_FOCUSHELPER}
    )

add_executable(screensaver-focus-helper ${focushelper_SRCS})
target_link_libraries(screensaver-focus-helper Qt5::Core  Qt5::Widgets Qt5::X11Extras ${X11_LIBRARIES} ${XCB_LIBRARIES})

add_subdirectory(VirtualKeyboard)
add_subdirectory(screensaver)
add_subdirectory(ukccplugins)

add_dependencies(ukui-screensaver-dialog VirtualKeyboard ukui-screensaver-default screensaver-default Screensaver)

install(TARGETS
    ukui-screensaver-dialog
    ukui-screensaver-backend
    ukui-screensaver-command
    ukui-screensaver-checkpass
    DESTINATION bin)

install(TARGETS
        screensaver-focus-helper
        DESTINATION lib/ukui-screensaver)

install(DIRECTORY assets/badges DESTINATION /usr/share/ukui-screensaver/images/)
