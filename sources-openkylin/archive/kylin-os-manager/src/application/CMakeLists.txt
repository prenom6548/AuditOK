project(kylin-os-manager LANGUAGES CXX)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(KF5WindowSystem)

set(SRCS
	"${TOP_DIR}/interface/kom_application_interface.cpp"
	"${SRC_DIR}/main.cpp"
	"${SRC_DIR}/mainwindow.cpp"
	"${SRC_DIR}/setup_widget.cpp"
)

set (COMPONENT
    "${TOP_DIR}/plugins/fault_diagnosis/component/text_label.cpp"
    "${TOP_DIR}/plugins/fault_diagnosis/component/utils.cpp"
)

file(GLOB TRANSLATION_FILES "${SRC_DIR}/translations/*.ts")
set_source_files_properties(${TRANSLATION_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATION_FILES})

include_directories(${TOP_DIR}/kom)
include_directories(${TOP_DIR}/interface)
include_directories(${TOP_DIR}/3rdparty/kyplugin)
include_directories(${TOP_DIR}/plugins/fault_diagnosis/component/)
include_directories(${SRC_DIR})

add_executable(${PROJECT_NAME} ${SRCS} ${COMPONENT} ${QM_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_libraries(${PROJECT_NAME} PRIVATE kyplugin)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE dl)
target_link_libraries(${PROJECT_NAME} PRIVATE KF5::WindowSystem)

pkg_check_modules(KYSDK_MODULE kysdk-alm kysdk-qtwidgets kysdk-ukenv kysdk-diagnostics kysdk-log kysdk-waylandhelper)
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
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-qtwidgets/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-ukenv/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-diagnostics/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-log/)
    target_include_directories(${PROJECT_NAME} PRIVATE ${COMPATIBILITY_DIR}/kysdk-waylandhelper/)
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

