cmake_minimum_required(VERSION 3.16)

project(DriverManagerTool)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(SERVICE_SUPPORT_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(${PROJECT_NAME} SHARED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets DBus Core LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)

include_directories(${TOP_DIR}/interface/)
include_directories(${TOP_DIR}/3rdparty/kyplugin/)

set(SRCS
        ${SERVICE_SUPPORT_TOP_DIR}/plugin.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/connector.cpp
        ${TOP_DIR}/interface/kom_application_interface.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfowidget.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/soundcardinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/networkcardinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/hardwareinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/graphiccardinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/bluetoothinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/driverinfo/driverinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/driverinfo/drivermodeinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfogetter.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/pcinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/mainwidget.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/driverinstallwidget.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/selectionwidget.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/loadingwidget.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/include/globalsignal.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/include/custompushbutton.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/deviceitem.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/driveruninstallmainwidget.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/getinfothread.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/include/customlabel.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/driveroperation/aptinstaller.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/driveruninstallwidget.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/nodriveruninstallpage.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/uninstallitem.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/driveroperation/uninstaller.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/cpuinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/treeitem.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/memoryinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/baseboardinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/diskinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/monitorinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/keyboardinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/mouseinfo.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/driverinstallmainwidget.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/drivergetfailwidget.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfomainwidget.cpp
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfogetfailwidget.cpp
)

set(HEADERS
        ${SERVICE_SUPPORT_TOP_DIR}/plugin.h
        ${TOP_DIR}/interface/kom_application_interface.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfowidget.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/soundcardinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/networkcardinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/hardwareinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/graphiccardinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/bluetoothinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/driverinfo/driverinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/driverinfo/drivermodeinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfogetter.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/pcinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/include/common.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/mainwidget.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/driverinstallwidget.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/selectionwidget.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/loadingwidget.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/include/globalsignal.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/include/custompushbutton.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/deviceitem.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/driveruninstallmainwidget.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/getinfothread.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/include/customlabel.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/driveroperation/aptinstaller.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/driveruninstallwidget.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/nodriveruninstallpage.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/uninstallitem.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/driveroperation/uninstaller.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/cpuinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/treeitem.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/memoryinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/baseboardinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/diskinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/monitorinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/keyboardinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfo/mouseinfo.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/drivergetfailwidget.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfomainwidget.h
        ${SERVICE_SUPPORT_TOP_DIR}/src/hardwareinfogetfailwidget.h
)

SET(UI_PICTURE ${SERVICE_SUPPORT_TOP_DIR}/src/img_src/image.qrc)

set(TRANSLATIONS
        "${SERVICE_SUPPORT_TOP_DIR}/translations/kylin-os-manager-driver-controller_zh_CN.ts"
        "${SERVICE_SUPPORT_TOP_DIR}/translations/kylin-os-manager-driver-controller_bo_CN.ts"
        "${SERVICE_SUPPORT_TOP_DIR}/translations/kylin-os-manager-driver-controller_mn.ts"
        "${SERVICE_SUPPORT_TOP_DIR}/translations/kylin-os-manager-driver-controller_ug.ts"
        "${SERVICE_SUPPORT_TOP_DIR}/translations/kylin-os-manager-driver-controller_kk.ts"
        "${SERVICE_SUPPORT_TOP_DIR}/translations/kylin-os-manager-driver-controller_ky.ts"
)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${HEADERS} ${UI_PICTURE} ${QM_FILES})

include_directories(${SERVICE_SUPPORT_TOP_DIR}/../../kom/)
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
