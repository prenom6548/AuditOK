cmake_minimum_required(VERSION 3.16)

project(kylin-system-cleanup LANGUAGES CXX)

set(SYSTEM_CLEANUP_DIR ${CMAKE_CURRENT_LIST_DIR})
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
add_definitions(-DQT_NO_KEYWORDS)
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

find_package(Qt5 REQUIRED COMPONENTS DBus LinguistTools)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

#Gsetting
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::QGSETTINGS)


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

# GIO
pkg_check_modules(GIO gio-2.0)
target_include_directories(${PROJECT_NAME} PRIVATE ${GIO_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${GIO_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GIO_LIBRARIES})

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

file(GLOB TRANSLATION_TS_FILES "${SYSTEM_CLEANUP_DIR}/translations/*.ts")
set_source_files_properties(${TRANSLATION_TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATION_TS_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${SYSTEM_CLEANUP_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kylin-os-manager/bin/)
install(FILES ${QM_FILES} DESTINATION /usr/share/kylin-os-manager/treasure-chest/kylin-system-cleanup/translations/)
install(FILES kylin-system-cleanup.conf DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/plugins/)
