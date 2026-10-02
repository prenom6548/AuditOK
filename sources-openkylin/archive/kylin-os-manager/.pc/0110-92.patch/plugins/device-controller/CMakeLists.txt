cmake_minimum_required(VERSION 3.16)

project(DriverManagerTool)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(${PROJECT_NAME} SHARED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets DBus Core LinguistTools Sql REQUIRED)
find_package(PkgConfig REQUIRED)

include_directories(${TOP_DIR}/interface/)
include_directories(${TOP_DIR}/3rdparty/kyplugin/)

set(SRCS
        ${CMAKE_CURRENT_LIST_DIR}/plugin.cpp
        ${CMAKE_CURRENT_LIST_DIR}/connector.cpp
        ${TOP_DIR}/interface/kom_application_interface.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfowidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/basiccontentwidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hwwidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/soundcardinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/networkcardinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/hardwareinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/graphiccardinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/bluetoothinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/driverinfo/driverinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/driverinfo/drivermodeinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfogetter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/pcinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/include/customtreewidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/mainwidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/driverinstallwidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/selectionwidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/loadingwidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/include/globalsignal.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/include/custompushbutton.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/include/customexpandbutton.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/deviceitem.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/getinfothread.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/include/customlabel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/driveroperation/aptinstaller.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/uninstallitem.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/driveroperation/uninstaller.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/deviceinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/cpuinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/treeitem.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/memoryinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/baseboardinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/diskinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/monitorinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/keyboardinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/mouseinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/batteryinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/faninfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/cdinfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/camerainfo.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/driverinstallmainwidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/drivergetfailwidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfomainwidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfogetfailwidget.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hotplug/hotplugclient.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/hotplug/devicemonitorthread.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database/drivermanagerdatabase.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/include/text_label.cpp
)

set(HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/plugin.h
        ${TOP_DIR}/interface/kom_application_interface.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfowidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/basiccontentwidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hwwidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/soundcardinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/networkcardinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/hardwareinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/graphiccardinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/bluetoothinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/driverinfo/driverinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/driverinfo/drivermodeinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfogetter.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/pcinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/include/common.h
        ${CMAKE_CURRENT_LIST_DIR}/src/include/customtreewidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/mainwidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/driverinstallwidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/selectionwidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/loadingwidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/include/globalsignal.h
        ${CMAKE_CURRENT_LIST_DIR}/src/include/custompushbutton.h
        ${CMAKE_CURRENT_LIST_DIR}/src/include/customexpandbutton.h
        ${CMAKE_CURRENT_LIST_DIR}/src/deviceitem.h
        ${CMAKE_CURRENT_LIST_DIR}/src/getinfothread.h
        ${CMAKE_CURRENT_LIST_DIR}/src/include/customlabel.h
        ${CMAKE_CURRENT_LIST_DIR}/src/driveroperation/aptinstaller.h
        ${CMAKE_CURRENT_LIST_DIR}/src/uninstallitem.h
        ${CMAKE_CURRENT_LIST_DIR}/src/driveroperation/uninstaller.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/deviceinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/cpuinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/treeitem.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/memoryinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/baseboardinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/diskinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/monitorinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/keyboardinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/mouseinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/batteryinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/faninfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/cdinfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfo/camerainfo.h
        ${CMAKE_CURRENT_LIST_DIR}/src/drivergetfailwidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfomainwidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hardwareinfogetfailwidget.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hotplug/hotplugclient.h
        ${CMAKE_CURRENT_LIST_DIR}/src/hotplug/devicemonitorthread.h
        ${CMAKE_CURRENT_LIST_DIR}/src/database/drivermanagerdatabase.h
        ${CMAKE_CURRENT_LIST_DIR}/src/include/text_label.h
)

SET(UI_PICTURE ${CMAKE_CURRENT_LIST_DIR}/src/img_src/image.qrc)

set(TRANSLATIONS
        "${CMAKE_CURRENT_LIST_DIR}/translations/kylin-os-manager-device-controller_zh_CN.ts"
        "${CMAKE_CURRENT_LIST_DIR}/translations/kylin-os-manager-device-controller_zh_HK.ts"
        "${CMAKE_CURRENT_LIST_DIR}/translations/kylin-os-manager-device-controller_bo_CN.ts"
        "${CMAKE_CURRENT_LIST_DIR}/translations/kylin-os-manager-device-controller_mn.ts"
        "${CMAKE_CURRENT_LIST_DIR}/translations/kylin-os-manager-device-controller_ug.ts"
        "${CMAKE_CURRENT_LIST_DIR}/translations/kylin-os-manager-device-controller_kk.ts"
        "${CMAKE_CURRENT_LIST_DIR}/translations/kylin-os-manager-device-controller_ky.ts"
)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HEADERS} ${UI_PICTURE} ${QM_FILES})

include_directories(${CMAKE_CURRENT_LIST_DIR}/../../kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/interface)
target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/3rdparty/kyplugin)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Core Qt5::DBus Qt5::Sql "-ludev")

find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::QGSETTINGS)

# 添加SDK模块
function(find_kysdk_package prj_name kysdk_package_name)
    pkg_check_modules(${kysdk_package_name}_PKG ${kysdk_package_name})
    target_include_directories(${PROJECT_NAME} PRIVATE ${${kysdk_package_name}_PKG_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${${kysdk_package_name}_PKG_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${${kysdk_package_name}_PKG_LIBRARIES})
endfunction()

find_kysdk_package(${PROJECT_NAME} kysdk-qtwidgets)
find_kysdk_package(${PROJECT_NAME} kysdk-ukenv)
find_kysdk_package(${PROJECT_NAME} kysdk-diagnostics)

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/kylin-os-manager/plugins/)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/translations/)
