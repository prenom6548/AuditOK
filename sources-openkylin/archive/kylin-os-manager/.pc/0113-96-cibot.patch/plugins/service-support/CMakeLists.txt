cmake_minimum_required(VERSION 3.16)

project(service-support)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(SERVICE_SUPPORT_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(${PROJECT_NAME} SHARED)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g -rdynamic)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

find_package(PkgConfig)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
include_directories(${GSETTINGS_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::QGSETTINGS)

pkg_check_modules(KYSDK_MODULE kysdk-qtwidgets kysdk-ukenv kysdk-systime)
if (${KYSDK_MODULE_FOUND})
    target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARIES})
else()
    message("========================= Use Compatibility: ${PROJECT_NAME} =========================")
    set(COMPATIBILITY_NAME compatibility)
    set(COMPATIBILITY_DIR ${TOP_DIR}/${COMPATIBILITY_NAME})
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-qtwidgets/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-ukenv/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-systime/)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_NAME})
endif()

set(SERVICE_SUPPORT_BACKEND service-support-backend)
add_subdirectory(${SERVICE_SUPPORT_BACKEND})
target_include_directories(${PROJECT_NAME} PRIVATE ${SERVICE_SUPPORT_TOP_DIR}/${SERVICE_SUPPORT_BACKEND})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SERVICE_SUPPORT_BACKEND})

find_package(Qt5 REQUIRED COMPONENTS DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

find_package(Qt5 COMPONENTS WebEngineWidgets)
if (${Qt5WebEngineWidgets_FOUND})
    add_definitions(-DHAS_WEBENGINE)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::WebEngineWidgets)
endif()

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

include_directories(${SERVICE_SUPPORT_TOP_DIR}/../../kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

qt5_add_resources(qrc_FILES res.qrc)

set(SRCS
     "${SERVICE_SUPPORT_TOP_DIR}/connector.cpp"
     "${SERVICE_SUPPORT_TOP_DIR}/plugin.cpp"
     "${TOP_DIR}/interface/kom_application_interface.cpp"
     "${SERVICE_SUPPORT_TOP_DIR}/UI/uiservicesupport.cpp"
     "${SERVICE_SUPPORT_TOP_DIR}/UI/uiproblemfeedback.cpp"
     "${SERVICE_SUPPORT_TOP_DIR}/UI/uiproblemfeedbackdialog.cpp"
     "${SERVICE_SUPPORT_TOP_DIR}/UI/uiselfservice.cpp"
     "${SERVICE_SUPPORT_TOP_DIR}/UI/uiserviceonline.cpp"
     "${SERVICE_SUPPORT_TOP_DIR}/UI/uihistoryfeedback.cpp"
     "${SERVICE_SUPPORT_TOP_DIR}/UI/paginationwid.cpp"
     "${SERVICE_SUPPORT_TOP_DIR}/UI/uimainpage.cpp"
     "${SERVICE_SUPPORT_TOP_DIR}/constant.h"
    ${qrc_FILES}
    )

target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/interface)
target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/3rdparty/kyplugin)
target_include_directories(${PROJECT_NAME} PRIVATE ${SERVICE_SUPPORT_TOP_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/kylin-os-manager/plugins/)

set(TRANSLATIONS
       "${SERVICE_SUPPORT_TOP_DIR}/translations/kom-service-support_zh_CN.ts"
       "${SERVICE_SUPPORT_TOP_DIR}/translations/kom-service-support_zh_HK.ts"
       "${SERVICE_SUPPORT_TOP_DIR}/translations/kom-service-support_bo_CN.ts"
       "${SERVICE_SUPPORT_TOP_DIR}/translations/kom-service-support_mn.ts"
       "${SERVICE_SUPPORT_TOP_DIR}/translations/kom-service-support_ug.ts"
       "${SERVICE_SUPPORT_TOP_DIR}/translations/kom-service-support_kk.ts"
       "${SERVICE_SUPPORT_TOP_DIR}/translations/kom-service-support_ky.ts"
        )
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${QM_FILES})
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/translations/)

install(FILES ${SERVICE_SUPPORT_TOP_DIR}/script/architecture.py DESTINATION /usr/share/kylin-os-manager/script)
install(FILES ${SERVICE_SUPPORT_TOP_DIR}/script/machine.py DESTINATION /usr/share/kylin-os-manager/script)
install(FILES ${SERVICE_SUPPORT_TOP_DIR}/script/node.py DESTINATION /usr/share/kylin-os-manager/script)
install(FILES ${SERVICE_SUPPORT_TOP_DIR}/script/release.py DESTINATION /usr/share/kylin-os-manager/script)


add_subdirectory(service-support-mini)
