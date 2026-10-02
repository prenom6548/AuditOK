cmake_minimum_required(VERSION 3.16)

project(win-data-migration LANGUAGES CXX)

set(WIN_DATA_MIGRATION_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)

add_executable(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)

find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

include_directories(${WIN_DATA_MIGRATION_DIR}/../../kom/)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)

find_package(PkgConfig)
pkg_check_modules(KYSDK_MODULE kysdk-alm kysdk-kabase kysdk-qtwidgets kysdk-ukenv kysdk-diagnostics)

target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARIES})

pkg_check_modules(SMBCLIENT_PKG smbclient)
target_include_directories(${PROJECT_NAME} PRIVATE ${SMBCLIENT_PKG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${SMBCLIENT_PKG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SMBCLIENT_PKG_LIBRARIES})

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_definitions(-DDEBUG_MODE)
endif()



qt5_add_resources(qrc_FILES res.qrc)

set(SRCS
	${WIN_DATA_MIGRATION_DIR}/main.cpp
        ${WIN_DATA_MIGRATION_DIR}/constant.h
        ${WIN_DATA_MIGRATION_DIR}/constant.cpp
        ${WIN_DATA_MIGRATION_DIR}/dialogmessage.h
        ${WIN_DATA_MIGRATION_DIR}/dialogmessage.cpp
        ${WIN_DATA_MIGRATION_DIR}/dialogmigrationlogs.h
        ${WIN_DATA_MIGRATION_DIR}/dialogmigrationlogs.cpp
        ${WIN_DATA_MIGRATION_DIR}/dialogviewdelegate.h
        ${WIN_DATA_MIGRATION_DIR}/dialogviewdelegate.cpp
        ${WIN_DATA_MIGRATION_DIR}/dialogcommon.h
        ${WIN_DATA_MIGRATION_DIR}/dialogcommon.cpp
        ${WIN_DATA_MIGRATION_DIR}/widgetcommon.h
        ${WIN_DATA_MIGRATION_DIR}/widgetcommon.cpp
        ${WIN_DATA_MIGRATION_DIR}/widgetconnect.h
        ${WIN_DATA_MIGRATION_DIR}/widgetconnect.cpp
        ${WIN_DATA_MIGRATION_DIR}/widgetmigration.h
        ${WIN_DATA_MIGRATION_DIR}/widgetmigration.cpp
	${WIN_DATA_MIGRATION_DIR}/applogic.h
        ${WIN_DATA_MIGRATION_DIR}/applogic.cpp
        ${qrc_FILES})

set(TRANSLATIONS
    "${WIN_DATA_MIGRATION_DIR}/translations/win-data-migration_zh_CN.ts"
    "${WIN_DATA_MIGRATION_DIR}/translations/win-data-migration_bo_CN.ts"
    "${WIN_DATA_MIGRATION_DIR}/translations/win-data-migration_mn.ts"
    "${WIN_DATA_MIGRATION_DIR}/translations/win-data-migration_ug.ts"
    "${WIN_DATA_MIGRATION_DIR}/translations/win-data-migration_kk.ts"
    "${WIN_DATA_MIGRATION_DIR}/translations/win-data-migration_ky.ts"
    "${WIN_DATA_MIGRATION_DIR}/translations/win-data-migration_zh_HK.ts"
    "${WIN_DATA_MIGRATION_DIR}/translations/win-data-migration_de.ts"
    "${WIN_DATA_MIGRATION_DIR}/translations/win-data-migration_es.ts"
    "${WIN_DATA_MIGRATION_DIR}/translations/win-data-migration_fr.ts"
    )
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

target_include_directories(${PROJECT_NAME} PRIVATE ${WIN_DATA_MIGRATION_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS} ${QM_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/share/kylin-os-manager/win-data-migration/)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/win-data-migration/translations/)
install(FILES res/icon.svg DESTINATION /usr/share/kylin-os-manager/win-data-migration/icon/)
