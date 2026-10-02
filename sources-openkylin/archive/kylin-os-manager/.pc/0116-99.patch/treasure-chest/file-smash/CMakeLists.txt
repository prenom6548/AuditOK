cmake_minimum_required(VERSION 3.5)

project(kylin-file-crush LANGUAGES CXX)

set(CURRENT_DIR ${CMAKE_CURRENT_LIST_DIR})
add_definitions(-DQT_NO_KEYWORDS)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)

execute_process(COMMAND bash ${CURRENT_DIR}/translations/generate_translations_ts.sh
    WORKING_DIRECTORY ${CURRENT_DIR})
execute_process(COMMAND bash ${CURRENT_DIR}/translations/generate_translations_qm.sh
    WORKING_DIRECTORY ${CURRENT_DIR})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g -rdynamic)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

find_package(Qt5 COMPONENTS Widgets DBus REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets PRIVATE Qt5::DBus)


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

target_include_directories(${PROJECT_NAME} PRIVATE ${GIO_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${GIO_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GIO_LIBRARIES})
pkg_check_modules(GIO_UNIX gio-unix-2.0)
target_include_directories(${PROJECT_NAME} PRIVATE ${GIO_UNIX_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${GIO_UNIX_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GIO_UNIX_LIBRARIES})

file(GLOB SRCS
    "filewipe.cpp"
    "main.cpp"
    "shreddialog.cpp"
#    "shredmanager.cpp"
    "alertdialog.cpp"
    "toolkits.cpp"
    "mytitlebar.cpp"
    "systembutton.cpp"
    "kylineditbutton.cpp"
    "mytristatebutton.cpp"
    "shredqthread.cpp"
    "kalertdialog.cpp"
    "ksuredialog.cpp"
    "shredfailinfodialog.cpp"
    "cmyfiledialog.cpp"

    "filewipe.h"
    "shreddialog.h"
#    "shredmanager.h"
    "toolkits.h"
    "alertdialog.h"
    "utils.h"
    "toolkits.h"
    "mytitlebar.h"
    "systembutton.h"
    "kylineditbutton.h"
    "mytristatebutton.h"
    "shredqthread.h"
    "kalertdialog.h"
    "ksuredialog.h"
    "shredfailinfodialog.h"
    "cmyfiledialog.h"

    "img.qrc"

    "*.ui")

target_include_directories(${PROJECT_NAME} PRIVATE ${CURRENT_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

file(GLOB TRANS
    "${CURRENT_DIR}/translations/kylin-file-crush_zh_CN.qm"
    "${CURRENT_DIR}/translations/kylin-file-crush_zh_HK.qm"
    "${CURRENT_DIR}/translations/kylin-file-crush_bo_CN.qm"
    "${CURRENT_DIR}/translations/kylin-file-crush_mn.qm"
    "${CURRENT_DIR}/translations/kylin-file-crush_ug.qm"
    "${CURRENT_DIR}/translations/kylin-file-crush_kk.qm"
    "${CURRENT_DIR}/translations/kylin-file-crush_ky.qm"
    )

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/share/kylin-os-manager/treasure-chest/kylin-file-crush/)
install(FILES ${TRANS} DESTINATION  /usr/share/kylin-os-manager/treasure-chest/kylin-file-crush/translations)
install(FILES ${CURRENT_DIR}/kylin-file-crush.conf DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/plugins/)
install(DIRECTORY ${CURRENT_DIR}/data/user_manual/ DESTINATION /usr/share/kylin-user-guide/data/guide/kylin-file-crush/)
