cmake_minimum_required(VERSION 3.16)
include(GNUInstallDirs)
project(ukui-qml-style-helper)

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
#find_package(KF5WindowSystem REQUIRED)
#find_package(KF5Wayland)

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
    "kyquickstyleitem.h"
    "kystylehelper.h"
    "qqc2styleplugin.h"
    "../qt5-ukui-platformtheme/platform-theme-fontdata.h"
    "../ukui-styles/readconfig.h"
    "../../libqt5-ukui-style/settings/ukui-style-settings.h"
#    "styleparameter/tooltip/tooltip.h"
#    "styleparameter/tooltip/shared-qml-engine.h"
#    "styleparameter/tooltip/shared-engine-component.h"
    "styleparameter/tokenparameter.h"
    "styleparameter/appparameter.h"
    "styleparameter/ukuilabel.h"
    "styleparameter/ukuiprogressbar.h"
    "styleparameter/ukuiswitch.h"
    "styleparameter/icon.h"
    "styleparameter/ukuibutton.h"
    "styleparameter/ukuimenu.h"
    "styleparameter/ukuiradiobutton.h"
    "styleparameter/ukuitabbar.h"
    "styleparameter/imageprovider.h"
    "styleparameter/ukuicheckbox.h"
    "styleparameter/ukuimenuitem.h"
    "styleparameter/ukuiscrollbar.h"
    "styleparameter/ukuitabbutton.h"
    "styleparameter/parsecolorinterface.h"
    "styleparameter/ukuipopupwindowhandle.h"
    "styleparameter/ukuipopup.h"
    "styleparameter/ukuitextfiled.h"
    "styleparameter/ukuicombobox.h"
    "styleparameter/ukuislider.h"
    "styleparameter/ukuitextarea.h"
    "styleparameter/ukuiheaderview.h"
    "styleparameter/ukuitoken.h"
    )
file(GLOB_RECURSE SRC_FILES
    "KyIcon.cpp"
    "kyquickstyleitem.cpp"
    "kystylehelper.cpp"
    "qqc2styleplugin.cpp"
    "../qt5-ukui-platformtheme/platform-theme-fontdata.cpp"
    "../ukui-styles/readconfig.cpp"
#    "styleparameter/tooltip/tooltip.cpp"
#    "styleparameter/tooltip/shared-qml-engine.cpp"
#    "styleparameter/tooltip/shared-engine-component.cpp"
    "styleparameter/tokenparameter.cpp"
    "styleparameter/appparameter.cpp"
    "styleparameter/ukuilabel.cpp"
    "styleparameter/ukuiprogressbar.cpp"
    "styleparameter/ukuiswitch.cpp"
    "styleparameter/icon.cpp"
    "styleparameter/ukuibutton.cpp"
    "styleparameter/ukuimenu.cpp"
    "styleparameter/ukuiradiobutton.cpp"
    "styleparameter/ukuitabbar.cpp"
    "styleparameter/imageprovider.cpp"
    "styleparameter/ukuicheckbox.cpp"
    "styleparameter/ukuimenuitem.cpp"
    "styleparameter/ukuiscrollbar.cpp"
    "styleparameter/ukuitabbutton.cpp"
    "styleparameter/ukuicombobox.cpp"
    "styleparameter/ukuipopup.cpp"
    "styleparameter/ukuislider.cpp"
    "styleparameter/ukuitextfiled.cpp"
    "styleparameter/parsecolorinterface.cpp"
    "styleparameter/ukuiitemdelegate.cpp"
    "styleparameter/ukuipopupwindowhandle.cpp"
    "styleparameter/ukuispinbox.cpp"
    "styleparameter/ukuitooltip.cpp"
    "styleparameter/ukuitextarea.cpp"
    "styleparameter/ukuiheaderview.cpp"
    "styleparameter/ukuitoken.cpp"
    )
file(GLOB_RECURSE OTHER_FILES qmldir)

SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
SOURCE_GROUP("Source Files" FILES ${SRC_FILES})
SOURCE_GROUP("other files" FILES ${OTHER_FILES})
#set(QRC_FILES qml/qml.qrc)

include_directories(../libqt5-ukui-style/)

add_library(ukui-qml-style-helper MODULE ${HEADER_FILES} ${SRC_FILES} ${OTHER_FILES} #[[${QRC_FILES}]])

target_link_libraries(ukui-qml-style-helper PRIVATE
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
    ${FONTCONFIG_LIBRARIES}
    ${EXTERNAL_LIBS}
    ${UKUIWINDOWHELPER_LIBRARIES}

#    KF5::WindowSystem
#    KF5::WaylandClient
    )

if(UNIX)
    set(TARGET_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/qqc2style/private/")

    MESSAGE("libukui-qml-style-helper TARGET_PATH: ${TARGET_PATH}")

    install(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PATH})
    install(FILES ${OTHER_FILES} DESTINATION "${TARGET_PATH}")
#    install(DIRECTORY "qml" DESTINATION "${TARGET_PATH}")
endif()
