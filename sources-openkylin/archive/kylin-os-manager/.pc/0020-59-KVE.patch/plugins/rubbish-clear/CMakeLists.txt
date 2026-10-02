cmake_minimum_required(VERSION 3.5)

project(rubbish-clear)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(RUBBISH_CLEAR_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_library(${PROJECT_NAME} SHARED)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g -rdynamic)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

find_package(PkgConfig REQUIRED)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

find_package(Qt5 COMPONENTS DBus REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

find_package(Qt5 COMPONENTS Svg REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Svg )


find_package(PkgConfig)
pkg_check_modules(KYSDK_MODULE kysdk-kabase kysdk-ukenv kysdk-diagnostics kysdk-qtwidgets)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARIES})

target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

pkg_check_modules(GIO-2.0 gio-2.0)
target_include_directories(${PROJECT_NAME} PRIVATE ${GIO-2.0_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${GIO-2.0_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GIO-2.0_LIBRARIES})

file(GLOB SRCS
    "clearmainwidget.cpp"
    "rubbishclearentr.cpp"
    "connecter.cpp"
    "clearwidgetkinditem.cpp"
    "selectcategorywidget.cpp"
    "selectlistitem.cpp"
    "selectlistwidget.cpp"
    "systembutton.cpp"
    "cleanerwidget.cpp"
    "cleandetailveiw.cpp"
    "selectwidget.cpp"
    "cleartrash.cpp"
    "publicstatus.cpp"
    "clearmanager.cpp"
    "kalertdialog.cpp"
    "toolutils.cpp"
    "custom_label.cpp"
    "common_ui.cpp"
    "${TOP_DIR}/plugins/common/frame.cpp"

    "clearmainwidget.h"
    "rubbishclearentr.h"
    "clearwidgetkinditem.h"
    "clearcommon.h"
    "selectcategorywidget.h"
    "selectlistitem.h"
    "selectlistwidget.h"
    "systembutton.h"
    "utils.h"
    "cleanerwidget.h"
    "cleandetailveiw.h"
    "selectwidget.h"
    "clownTips.h"
    "cleartrash.h"
    "publicstatus.h"
    "clearmanager.h"
    "kalertdialog.h"
    "toolutils.h"
    "custom_label.h"
    "common_ui.h"

    "dbus/*.cpp"
    "dbus/*.h"
    "res.qrc"
    "${TOP_DIR}/interface/kom_application_interface.cpp")

set(TRANSLATIONS
	"${RUBBISH_CLEAR_DIR}/translations/kylin-os-manager-rubbish-clear_zh_CN.ts"
	"${RUBBISH_CLEAR_DIR}/translations/kylin-os-manager-rubbish-clear_bo_CN.ts"
    "${RUBBISH_CLEAR_DIR}/translations/kylin-os-manager-rubbish-clear_mn.ts"
	"${RUBBISH_CLEAR_DIR}/translations/kylin-os-manager-rubbish-clear_ug.ts"
	"${RUBBISH_CLEAR_DIR}/translations/kylin-os-manager-rubbish-clear_kk.ts"
	"${RUBBISH_CLEAR_DIR}/translations/kylin-os-manager-rubbish-clear_ky.ts"
	"${RUBBISH_CLEAR_DIR}/translations/kylin-os-manager-rubbish-clear_zh_HK.ts"
	"${RUBBISH_CLEAR_DIR}/translations/kylin-os-manager-rubbish-clear_de.ts"
	"${RUBBISH_CLEAR_DIR}/translations/kylin-os-manager-rubbish-clear_es.ts"
	"${RUBBISH_CLEAR_DIR}/translations/kylin-os-manager-rubbish-clear_fr.ts")
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/plugins/common/)
target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/interface)
target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/3rdparty/kyplugin)
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/kylin-os-manager/plugins/)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/translations/)
