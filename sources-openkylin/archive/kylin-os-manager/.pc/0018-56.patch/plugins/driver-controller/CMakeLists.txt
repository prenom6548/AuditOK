cmake_minimum_required(VERSION 3.16)

project(DriverManagerTool)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(DRIVER_CONTROLLER_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})
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
        ${DRIVER_CONTROLLER_TOP_DIR}/plugin.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/connector.cpp
        ${TOP_DIR}/interface/kom_application_interface.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfowidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/soundcardinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/networkcardinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/hardwareinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/graphiccardinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/bluetoothinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driverinfo/driverinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driverinfo/drivermodeinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfogetter.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/pcinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/mainwidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driverinstallwidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/selectionwidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/loadingwidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/include/globalsignal.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/include/custompushbutton.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/deviceitem.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driveruninstallmainwidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/getinfothread.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/include/customlabel.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driveroperation/aptinstaller.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driveruninstallwidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/nodriveruninstallpage.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/uninstallitem.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driveroperation/uninstaller.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/cpuinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/treeitem.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/memoryinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/baseboardinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/diskinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/monitorinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/keyboardinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/mouseinfo.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driverinstallmainwidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/drivergetfailwidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfomainwidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfogetfailwidget.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hotplug/hotplugclient.cpp
        ${DRIVER_CONTROLLER_TOP_DIR}/src/database/drivermanagerdatabase.cpp
)

set(HEADERS
        ${DRIVER_CONTROLLER_TOP_DIR}/plugin.h
        ${TOP_DIR}/interface/kom_application_interface.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfowidget.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/soundcardinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/networkcardinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/hardwareinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/graphiccardinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/bluetoothinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driverinfo/driverinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driverinfo/drivermodeinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfogetter.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/pcinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/include/common.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/mainwidget.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driverinstallwidget.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/selectionwidget.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/loadingwidget.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/include/globalsignal.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/include/custompushbutton.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/deviceitem.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driveruninstallmainwidget.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/getinfothread.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/include/customlabel.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driveroperation/aptinstaller.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driveruninstallwidget.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/nodriveruninstallpage.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/uninstallitem.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/driveroperation/uninstaller.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/cpuinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/treeitem.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/memoryinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/baseboardinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/diskinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/monitorinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/keyboardinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfo/mouseinfo.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/drivergetfailwidget.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfomainwidget.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hardwareinfogetfailwidget.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/hotplug/hotplugclient.h
        ${DRIVER_CONTROLLER_TOP_DIR}/src/database/drivermanagerdatabase.h
)

SET(UI_PICTURE ${DRIVER_CONTROLLER_TOP_DIR}/src/img_src/image.qrc)

set(TRANSLATIONS
        "${DRIVER_CONTROLLER_TOP_DIR}/translations/kylin-os-manager-driver-controller_zh_CN.ts"
        "${DRIVER_CONTROLLER_TOP_DIR}/translations/kylin-os-manager-driver-controller_bo_CN.ts"
        "${DRIVER_CONTROLLER_TOP_DIR}/translations/kylin-os-manager-driver-controller_mn.ts"
        "${DRIVER_CONTROLLER_TOP_DIR}/translations/kylin-os-manager-driver-controller_ug.ts"
        "${DRIVER_CONTROLLER_TOP_DIR}/translations/kylin-os-manager-driver-controller_kk.ts"
        "${DRIVER_CONTROLLER_TOP_DIR}/translations/kylin-os-manager-driver-controller_ky.ts"
)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HEADERS} ${UI_PICTURE} ${QM_FILES})

include_directories(${DRIVER_CONTROLLER_TOP_DIR}/../../kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/interface)
target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/3rdparty/kyplugin)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Core Qt5::DBus)

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
