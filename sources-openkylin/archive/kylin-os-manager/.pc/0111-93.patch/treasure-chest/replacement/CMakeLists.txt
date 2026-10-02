cmake_minimum_required(VERSION 3.16)

project(replacement LANGUAGES CXX)

set(REPLACEMENT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)

add_subdirectory(3rdparty)

add_executable(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)

include_directories(${REPLACEMENT_DIR}/3rdparty/QtZeroConf/)
target_link_libraries(${PROJECT_NAME} PRIVATE QtZeroConf)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)

find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

include_directories(${REPLACEMENT_DIR}/../../kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
include_directories(${TOP_DIR}/daemons/service/)

find_package(Qt5 REQUIRED COMPONENTS DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

find_package(KF5WindowSystem REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE KF5::WindowSystem)

find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::QGSETTINGS)

find_package(PkgConfig)
pkg_check_modules(KYSDK_MODULE kysdk-alm kysdk-kabase kysdk-qtwidgets kysdk-ukenv kysdk-diagnostics kysdk-waylandhelper)
if (${KYSDK_MODULE_FOUND})
    target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARIES})
else()
    message("========================= Use Compatibility: ${PROJECT_NAME} =========================")
    set(COMPATIBILITY_NAME compatibility)
    set(COMPATIBILITY_DIR ${TOP_DIR}/${COMPATIBILITY_NAME})
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-alm/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-kabase/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-qtwidgets/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-ukenv/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-diagnostics/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-waylandhelper/)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_NAME})
endif()

pkg_check_modules(SMBCLIENT_PKG smbclient)
target_include_directories(${PROJECT_NAME} PRIVATE ${SMBCLIENT_PKG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${SMBCLIENT_PKG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SMBCLIENT_PKG_LIBRARIES})

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_definitions(-DDEBUG_MODE)
endif()

#添加架构判断
EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS"Architecture: ${ARCHITECTURE}" )

if(${ARCHITECTURE} STREQUAL "mips64" )
    add_definitions(-DIS_MIPS_TYPE)
elseif(${ARCHITECTURE} STREQUAL "mips64el" )
	add_definitions(-DIS_MIPS_TYPE)
endif()

qt5_add_resources(qrc_FILES res.qrc)

set(SRCS
	${REPLACEMENT_DIR}/main.cpp
	${REPLACEMENT_DIR}/globaldata.h
	${REPLACEMENT_DIR}/globaldata.cpp
    ${REPLACEMENT_DIR}/component/text_label.h
    ${REPLACEMENT_DIR}/component/text_label.cpp
    ${REPLACEMENT_DIR}/component/icon_label.h
    ${REPLACEMENT_DIR}/component/icon_label.cpp
    ${REPLACEMENT_DIR}/component/icon_widget.h
    ${REPLACEMENT_DIR}/component/icon_widget.cpp
    ${REPLACEMENT_DIR}/component/radiobutton.h
    ${REPLACEMENT_DIR}/component/radiobutton.cpp
    ${REPLACEMENT_DIR}/component/radio_widget.h
    ${REPLACEMENT_DIR}/component/radio_widget.cpp
    ${REPLACEMENT_DIR}/component/data_checkbox.h
    ${REPLACEMENT_DIR}/component/data_checkbox.cpp
    ${REPLACEMENT_DIR}/component/progressbar.h
    ${REPLACEMENT_DIR}/component/progressbar.cpp
    ${REPLACEMENT_DIR}/component/status_widget.h
    ${REPLACEMENT_DIR}/component/status_widget.cpp
    ${REPLACEMENT_DIR}/component/utils.h
    ${REPLACEMENT_DIR}/component/utils.cpp
    ${REPLACEMENT_DIR}/view/homepagewidget.h
    ${REPLACEMENT_DIR}/view/homepagewidget.cpp
    ${REPLACEMENT_DIR}/view/mainwidget.h
    ${REPLACEMENT_DIR}/view/mainwidget.cpp
    ${REPLACEMENT_DIR}/view/choosewidget.h
    ${REPLACEMENT_DIR}/view/choosewidget.cpp
    ${REPLACEMENT_DIR}/view/choosedevicewidget.h
    ${REPLACEMENT_DIR}/view/choosedevicewidget.cpp
    ${REPLACEMENT_DIR}/view/choosesyswidget.h
    ${REPLACEMENT_DIR}/view/choosesyswidget.cpp
    ${REPLACEMENT_DIR}/view/senddatawidget.h
    ${REPLACEMENT_DIR}/view/senddatawidget.cpp
    ${REPLACEMENT_DIR}/view/sendoldsysinfo.h
    ${REPLACEMENT_DIR}/view/sendoldsysinfo.cpp
    ${REPLACEMENT_DIR}/view/acceptchoosedevice.h
    ${REPLACEMENT_DIR}/view/acceptchoosedevice.cpp
    ${REPLACEMENT_DIR}/view/acceptdatawidget.h
    ${REPLACEMENT_DIR}/view/acceptdatawidget.cpp
    ${REPLACEMENT_DIR}/view/acceptdatastatus.h
    ${REPLACEMENT_DIR}/view/acceptdatastatus.cpp
    ${REPLACEMENT_DIR}/view/acceptchoosedata.h
    ${REPLACEMENT_DIR}/view/acceptchoosedata.cpp
    ${REPLACEMENT_DIR}/view/acceptmigrationdata.h
    ${REPLACEMENT_DIR}/view/acceptmigrationdata.cpp
    ${REPLACEMENT_DIR}/view/acceptmigrationappdata.h
    ${REPLACEMENT_DIR}/view/acceptmigrationappdata.cpp
    ${REPLACEMENT_DIR}/view/acceptrebootwidget.h
    ${REPLACEMENT_DIR}/view/acceptrebootwidget.cpp

    ${qrc_FILES})

set(TRANSLATIONS
    "${REPLACEMENT_DIR}/translations/replacement_zh_CN.ts"
    "${REPLACEMENT_DIR}/translations/replacement_zh_HK.ts"
    "${REPLACEMENT_DIR}/translations/replacement_bo_CN.ts"
    "${REPLACEMENT_DIR}/translations/replacement_mn.ts"
    "${REPLACEMENT_DIR}/translations/replacement_ug.ts"
    "${REPLACEMENT_DIR}/translations/replacement_kk.ts"
    "${REPLACEMENT_DIR}/translations/replacement_ky.ts"
    )
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

target_include_directories(${PROJECT_NAME} PRIVATE ${DATA_EXCHANGE_ASSISTANT_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/share/kylin-os-manager/replacement/)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/replacement/translations/)
install(FILES res/icon.svg DESTINATION /usr/share/kylin-os-manager/replacement/icon/)
install(FILES data/appblacklist.json DESTINATION /usr/share/kylin-os-manager/replacement/data/)
install(FILES data/ReplacementDataExcludeFile DESTINATION /usr/share/kylin-os-manager/replacement/data/)
install(FILES data/etcPolicy.yaml DESTINATION /usr/share/kylin-os-manager/replacement/data/)
install(FILES data/appClassification.yaml DESTINATION /usr/share/kylin-os-manager/replacement/data/)
install(FILES ${REPLACEMENT_DIR}/data/replacement.desktop DESTINATION /usr/share/applications/)
