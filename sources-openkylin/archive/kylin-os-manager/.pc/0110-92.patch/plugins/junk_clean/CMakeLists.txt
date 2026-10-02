project(junk_clean LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g")

set(TOP_DIR "${CMAKE_CURRENT_LIST_DIR}/../../")
set(COMPONENT_DIR "${CMAKE_CURRENT_LIST_DIR}/../fault_diagnosis/component/")

add_definitions(-DQT_NO_KEYWORDS)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets Xml DBus LinguistTools REQUIRED)

set(SRCS
    "${TOP_DIR}/interface/kom_application_interface.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../common/frame.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/connecter.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/junk_clean.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/clean_up_group_widget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/clean_up_entry_widget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/junk_entry_widget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/clean_up_service.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/clean_up_finish_widget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/homepage.cpp")

set(COMPONENT_SRCS
    "${COMPONENT_DIR}/icon_label.cpp"
    "${COMPONENT_DIR}/text_label.cpp"
    "${COMPONENT_DIR}/utils.cpp")

set(CLEANER_SRCS
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/cleaner.h"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/cleaner_manager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/installation_package_cleaner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/file_trace_cleaner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/qax_browser_cleaner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/qax_browser_cookies_cleaner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/command_line_trace_cleaner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/log_cleaner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/residual_cleaner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/system_memory_cleaner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/thumbnail_cleaner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/trash_cleaner.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cleaner/wechat_cleaner.cpp")

set(TRANSLATIONS
    "${CMAKE_CURRENT_LIST_DIR}/translations/junk-clean_zh_CN.ts"
    "${CMAKE_CURRENT_LIST_DIR}/translations/junk-clean_ug.ts"
    "${CMAKE_CURRENT_LIST_DIR}/translations/junk-clean_kk.ts"
    "${CMAKE_CURRENT_LIST_DIR}/translations/junk-clean_ky.ts"
    "${CMAKE_CURRENT_LIST_DIR}/translations/junk-clean_bo_CN.ts"
    "${CMAKE_CURRENT_LIST_DIR}/translations/junk-clean_mn.ts"
    "${CMAKE_CURRENT_LIST_DIR}/translations/junk-clean_zh_HK.ts")
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION "${CMAKE_BINARY_DIR}/translations")
qt5_add_translation(QM_FILES ${TRANSLATIONS})

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/cleaner/)
include_directories(${COMPONENT_DIR})
include_directories(${TOP_DIR}/kom/)
include_directories(${TOP_DIR}/interface/)
include_directories(${TOP_DIR}/3rdparty/kyplugin)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../common/)

add_library(${PROJECT_NAME} SHARED ${SRCS} ${COMPONENT_SRCS} ${CLEANER_SRCS} ${QM_FILES} ${CMAKE_CURRENT_LIST_DIR}/resource.qrc)
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::Xml Qt5::DBus)

pkg_check_modules(GIO-2.0 gio-2.0)
include_directories(${GIO-2.0_INCLUDE_DIRS})
link_directories(${GIO-2.0_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GIO-2.0_LIBRARIES})

pkg_check_modules(KYSDK_QTWIDGETS kysdk-qtwidgets)
if (${KYSDK_QTWIDGETS_FOUND})
    include_directories(${KYSDK_QTWIDGETS_INCLUDE_DIRS})
    link_directories(${KYSDK_QTWIDGETS_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_QTWIDGETS_LIBRARIES})
else ()
    include_directories(${TOP_DIR}/compatibility/kysdk-qtwidgets/)
    target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

pkg_check_modules(KYSDK_UKENV kysdk-ukenv)
if (${KYSDK_UKENV_FOUND})
    include_directories(${KYSDK_UKENV_INCLUDE_DIRS})
    link_directories(${KYSDK_UKENV_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${KYSDK_UKENV_LIBRARIES})
else ()
    include_directories(${TOP_DIR}/compatibility/kysdk-ukenv)
    target_link_libraries(${PROJECT_NAME} PRIVATE compatibility)
endif ()

install(TARGETS ${PROJECT_NAME} DESTINATION /opt/kylin-os-manager/plugins/)
install(FILES ${QM_FILES} DESTINATION /usr/share/kylin-os-manager/junk-clean/translations/)
