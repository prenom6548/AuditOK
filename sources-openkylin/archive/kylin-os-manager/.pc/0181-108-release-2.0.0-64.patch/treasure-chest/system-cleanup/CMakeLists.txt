cmake_minimum_required(VERSION 3.16)

project(kylin-system-cleanup LANGUAGES CXX)

set(SYSTEM_CLEANUP_DIR ${CMAKE_CURRENT_LIST_DIR})
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../../kom/)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)

target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/interface)
target_include_directories(${PROJECT_NAME} PRIVATE ${TOP_DIR}/3rdparty/kyplugin)

find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

#Gsetting
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::QGSETTINGS)

#peony
# pkg_search_module(自定义名  必需项  查找库名)
pkg_check_modules(PEONY REQUIRED peony)
target_include_directories(${PROJECT_NAME} PRIVATE ${PEONY_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${PEONY_LIBRARY_DIRS})
# 链接 ${PROJECT_NAME} ，同时需要链接 ${PEONY_INCLUDE_DIRS}
target_link_libraries(${PROJECT_NAME} PRIVATE ${PEONY_LIBRARIES})

find_package(KF5WindowSystem REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE KF5::WindowSystem)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

find_package(PkgConfig)
pkg_check_modules(KYSDK_MODULE kysdk-alm kysdk-kabase kysdk-qtwidgets)
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
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_NAME})
endif()

set(SRCS
	${SYSTEM_CLEANUP_DIR}/main.cpp
	${SYSTEM_CLEANUP_DIR}/mainwindow.cpp
    ${SYSTEM_CLEANUP_DIR}/homepage.cpp
    ${SYSTEM_CLEANUP_DIR}/scanfilepage.cpp
    ${SYSTEM_CLEANUP_DIR}/cleanupfinishpage.cpp
    ${SYSTEM_CLEANUP_DIR}/filemd5.cpp
    ${SYSTEM_CLEANUP_DIR}/scancanneldialog.cpp
    ${SYSTEM_CLEANUP_DIR}/tableheaderview.cpp
    ${SYSTEM_CLEANUP_DIR}/scanfileslistpage.cpp
    ${SYSTEM_CLEANUP_DIR}/component/radio_widget.cpp
    ${SYSTEM_CLEANUP_DIR}/component/text_label.cpp
    ${SYSTEM_CLEANUP_DIR}/component/utils.cpp
    ${SYSTEM_CLEANUP_DIR}/component/kom-ukui-gsettings.cpp
    ${SYSTEM_CLEANUP_DIR}/image.qrc
    )

set(TRANSLATIONS
	${SYSTEM_CLEANUP_DIR}/translations/komt-system-cleanup_zh_CN.ts
    ${SYSTEM_CLEANUP_DIR}/translations/komt-system-cleanup_bo_CN.ts
    ${SYSTEM_CLEANUP_DIR}/translations/komt-system-cleanup_mn.ts
    ${SYSTEM_CLEANUP_DIR}/translations/komt-system-cleanup_zh_HK.ts)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

target_include_directories(${PROJECT_NAME} PRIVATE ${SYSTEM_CLEANUP_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kylin-os-manager/bin/)
install(FILES ${QM_FILES} DESTINATION /usr/share/kylin-os-manager/treasure-chest/kylin-system-cleanup/translations/)
install(FILES kylin-system-cleanup.conf DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/plugins/)
