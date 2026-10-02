cmake_minimum_required(VERSION 3.16)

project(service-support-mini)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../)
set(SERVICE_SUPPORT_MINI_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SRCS
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/main.cpp"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/src/voluntarilywidget.cpp"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/src/passivitywidget.cpp"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/src/feedbackcommon.cpp"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/src/blacklist.cpp"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/res.qrc"
)

set(TRANSLATIONS
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/translations/kom-service-support-mini_zh_CN.ts"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/translations/kom-service-support-mini_zh_HK.ts"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/translations/kom-service-support-mini_bo_CN.ts"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/translations/kom-service-support-mini_mn.ts"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/translations/kom-service-support-mini_ug.ts"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/translations/kom-service-support-mini_kk.ts"
    "${SERVICE_SUPPORT_MINI_TOP_DIR}/translations/kom-service-support-mini_ky.ts"
)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILE ${TRANSLATIONS})

add_executable(${PROJECT_NAME} ${SRCS} ${QM_FILE})

pkg_check_modules(GLIB_2.0 REQUIRED glib-2.0)
include_directories(${GLIB_2.0_INCLUDE_DIRS})
link_directories(${GLIB_2.0_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLIB_2.0_LIBRARIES})

pkg_check_modules(KYSDK_MODULE kysdk-qtwidgets kysdk-kabase kysdk-diagnostics kysdk-ukenv)
if (${KYSDK_MODULE_FOUND})
    target_include_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_MODULE_LIBRARIES})
else()
    message("========================= Use Compatibility: ${PROJECT_NAME} =========================")
    set(COMPATIBILITY_NAME compatibility)
    set(COMPATIBILITY_DIR ${TOP_DIR}/${COMPATIBILITY_NAME})
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-qtwidgets/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-kabase/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-ukenv/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-diagnostics/)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_NAME})
endif()

find_package(PkgConfig)
pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
include_directories(${GSETTINGS_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::QGSETTINGS)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

find_package(KF5WindowSystem)
target_link_libraries(${PROJECT_NAME} PRIVATE KF5::WindowSystem)

target_include_directories(${PROJECT_NAME} PRIVATE ${SERVICE_SUPPORT_MINI_TOP_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${SERVICE_SUPPORT_MINI_TOP_DIR}/src)

target_include_directories(${PROJECT_NAME} PRIVATE ${SERVICE_SUPPORT_MINI_TOP_DIR}/../${SERVICE_SUPPORT_BACKEND})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SERVICE_SUPPORT_BACKEND})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kylin-os-manager/bin/)
install(FILES ${QM_FILE} DESTINATION  /usr/share/kylin-os-manager/translations/)

