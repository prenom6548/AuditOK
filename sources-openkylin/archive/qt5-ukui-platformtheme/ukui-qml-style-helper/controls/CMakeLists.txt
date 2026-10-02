cmake_minimum_required(VERSION 3.16)
include(GNUInstallDirs)
project(ukui-qml-style-helper-controls)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.66.0")

find_package(Qt5Quick)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
    DBus
    X11Extras
    Gui
)
#find_package(KF5Kirigami2 CONFIG REQUIRED COMPONENTS)
#find_package(KF5Config CONFIG REQUIRED COMPONENTS)
find_package(KF5WindowSystem REQUIRED)
find_package(KF5Wayland)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Qsettings REQUIRED gsettings-qt)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0 )
pkg_check_modules(UKUIWINDOWHELPER REQUIRED kysdk-ukuiwindowhelper)

set(EXTERNAL_LIBS "")

include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${Qsettings_INCLUDE_DIRS})
if (FONTCONFIG_FOUND)
    include_directories(${FONTCONFIG_INCLUDE_DIRS})
    link_directories(${FONTCONFIG_LIBRARY_DIRS})
endif()
if (UKUIWINDOWHELPER_FOUND)
    include_directories(${UKUIWINDOWHELPER_INCLUDE_DIRS})
    link_directories(${UKUIWINDOWHELPER_LIBRARY_DIRS})
endif()

file(GLOB_RECURSE HEADER_FILES
    "kyquickpadding_p.h"
    "KyIcon.h"
    "kybutton.h"
    "kydirhelper.h"
    "kdragfiledialog.h"
    "kydtbrush.h"
    "dtbrushutil.h"
    "../private/styleparameter/imageprovider.h"
    "../private/styleparameter/icon.h"
    "kyquickstyleitem.h"
    "kystylehelper.h"
    "qqc2styleplugin.h"
    "ukuiwindowmanager.h"
    "ukuiwindow.h"
    "ukuipopuphandle.h"
    "../private/styleparameter/ukuipopupwindowhandle.h"
    "kyquickDialog.cpp"
    "../../ukui-styles/readconfig.h"
    "../private/styleparameter/tokenparameter.h"
    )
file(GLOB_RECURSE SRC_FILES
    "KyIcon.cpp"
    "kybutton.cpp"
    "kydirhelper.cpp"
    "kdragfiledialog.cpp"
    "dtbrushutil.cpp"
    "../private/styleparameter/tokenparameter.cpp"
    "../private/styleparameter/imageprovider.cpp"
    "../../ukui-styles/readconfig.cpp"
    "kyquickstyleitem.cpp"
    "kystylehelper.cpp"
    "qqc2styleplugin.cpp"
    "ukuiwindowmanager.cpp"
    "ukuiwindow.cpp"
    "ukuipopuphandle.cpp"
    "../private/styleparameter/ukuipopupwindowhandle.cpp"
    "kyquickDialog.cpp"
    )
set(OTHER_FILES 
    qmldir
)

set(QML_SUPPORT_FILES
    qml/LunarCalendar.js
)

set(QML_FILES
    qml/KDialog.qml
    qml/KWindowButton.qml
    qml/KHeadBarMenuBtn.qml
    qml/KUninstallDialog.qml
    qml/KAboutDialog.qml
    qml/KBorderlessButton.qml
    qml/KBorderButton.qml
    qml/KSearchLineEdit.qml
    qml/KPopup.qml
    qml/KProgressCircle.qml
    qml/KProgressBar.qml
    qml/KSlider.qml
    qml/KSecurityLevelBar.qml
    qml/KColorButton.qml
    qml/KButtonBox.qml
    qml/KPushButton.qml
    qml/KBadge.qml
    qml/KHLineFrame.qml
    qml/KVLineFrame.qml
    qml/KBallonTip.qml
    qml/KLabel.qml
    qml/KBreadCrumbItem.qml
    qml/KBreadCrumb.qml
    qml/KDirTabBar.qml
    qml/KNavigationBar.qml
    qml/KWidget.qml
    qml/KProgressLabel.qml
    qml/KProgressDialog.qml
    qml/KInputDialog.qml
    qml/KDragWidget.qml
    qml/KBubbleWidget.qml
    qml/KDateEdit.qml
)

SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
SOURCE_GROUP("Source Files" FILES ${SRC_FILES})
SOURCE_GROUP("other files" FILES ${OTHER_FILES} ${QML_FILES} ${QML_SUPPORT_FILES})
#set(QRC_FILES qml/qml.qrc)
include_directories(../../libqt5-ukui-style/)
include_directories(../private/styleparameter/)
include_directories(../../ukui-styles/)

add_library(ukui-qml-style-helper-controls MODULE ${HEADER_FILES} ${SRC_FILES} ${OTHER_FILES} ${QML_FILES} ${QML_SUPPORT_FILES} #[[${QRC_FILES}]])

target_link_libraries(ukui-qml-style-helper-controls PRIVATE
#    KF5::ConfigCore
#    KF5::Kirigami2
    Qt5::Quick
    Qt5::Widgets
    Qt5::DBus
    Qt5::X11Extras
    Qt5::Gui
    Qt::GuiPrivate
    gsettings-qt
    qt5-ukui-style
    glib-2.0
    ${EXTERNAL_LIBS}
    ${UKUIWINDOWHELPER_LIBRARIES}
    KF5::WindowSystem
    KF5::WaylandClient
    )

if(UNIX)
    set(TARGET_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/platformtheme/controls/")

    MESSAGE("libukui-qml-style-helper-controls TARGET_PATH: ${TARGET_PATH}")

    install(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PATH})
    install(FILES ${OTHER_FILES} DESTINATION "${TARGET_PATH}")
    install(FILES ${QML_FILES} ${QML_SUPPORT_FILES} DESTINATION "${TARGET_PATH}/qml")
endif()
