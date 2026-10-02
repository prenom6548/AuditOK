project(kylin-os-manager LANGUAGES CXX)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)

set(SRCS
	"${SRC_DIR}/main.cpp"
	"${SRC_DIR}/mainwindow.cpp"
	"${TOP_DIR}/interface/kom_application_interface.cpp")

set(TRANSLATIONS
	"${SRC_DIR}/translations/kylin-os-manager_zh_CN.ts"
	"${SRC_DIR}/translations/kylin-os-manager_zh_HK.ts"
	"${SRC_DIR}/translations/kylin-os-manager_bo_CN.ts"
	"${SRC_DIR}/translations/kylin-os-manager_mn.ts"
	"${SRC_DIR}/translations/kylin-os-manager_ug.ts"
	"${SRC_DIR}/translations/kylin-os-manager_kk.ts"
	"${SRC_DIR}/translations/kylin-os-manager_ky.ts")
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

include_directories(${TOP_DIR}/kom)
include_directories(${TOP_DIR}/interface)
include_directories(${TOP_DIR}/3rdparty/kyplugin)
include_directories(${SRC_DIR})

add_executable(${PROJECT_NAME} ${SRCS} ${QM_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_libraries(${PROJECT_NAME} PRIVATE kyplugin)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE dl)

pkg_check_modules(KYSDK_MODULE kysdk-alm kysdk-kabase kysdk-qtwidgets kysdk-ukenv kysdk-diagnostics)
if (${KYSDK_MODULE_FOUND})
    add_definitions(-DHAS_KYSDK)
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
    target_link_libraries(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_NAME})
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${QM_FILES} DESTINATION  /usr/share/kylin-os-manager/translations/)
install(DIRECTORY ${TOP_DIR}/data/icons/ DESTINATION /usr/share/kylin-os-manager/icons/)

# 安装用户手册
if (ICBC_OPTION)
    install(DIRECTORY ${TOP_DIR}/data/environment/icbc/user-manual/ DESTINATION /usr/share/kylin-user-guide/data/guide/kylin-os-manager/)
else ()
    install(DIRECTORY ${TOP_DIR}/data/user-manual/ DESTINATION /usr/share/kylin-user-guide/data/guide/kylin-os-manager/)
endif ()

