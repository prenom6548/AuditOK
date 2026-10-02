project(home_page LANGUAGES CXX)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g")

add_definitions(-DQT_NO_KEYWORDS)

find_package(PkgConfig)
find_package(Qt5 COMPONENTS Widgets DBus Xml LinguistTools REQUIRED)

set(SRCS
    ${TOP_DIR}/interface/kom_application_interface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/connecter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/home_page.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/system_health_widget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/computer_junk_widget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/computer_treasure_chest_widget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/home_page_resource.qrc
)

set(COMMON
    ${TOP_DIR}/plugins/common/frame.cpp
)

set(COMPONENT
    ${TOP_DIR}/plugins/fault_diagnosis/component/text_label.cpp
    ${TOP_DIR}/plugins/fault_diagnosis/component/utils.cpp
)

set(DIAGNOSES
    ${TOP_DIR}/plugins/fault_diagnosis/diagnoses/diagnosis.h
    ${TOP_DIR}/plugins/fault_diagnosis/diagnoses/complete_machine.cpp
)

set(CLEANER
    ${TOP_DIR}/plugins/junk_clean/cleaner/cleaner.h
    ${TOP_DIR}/plugins/junk_clean/cleaner/cleaner_manager.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/boot_partition_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/command_line_trace_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/enterprise_wechat_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/file_trace_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/installation_package_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/log_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/qax_browser_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/qax_browser_cookies_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/qq_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/residual_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/system_memory_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/thumbnail_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/trash_cleaner.cpp
    ${TOP_DIR}/plugins/junk_clean/cleaner/wechat_cleaner.cpp
)

set(TRANSLATIONS
    ${CMAKE_CURRENT_LIST_DIR}/translations/home-page_zh_CN.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/home-page_zh_HK.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/home-page_bo_CN.ts
    ${CMAKE_CURRENT_LIST_DIR}/translations/home-page_mn.ts
)
set_source_files_properties(${TRANSLATIONS} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/translations/)
qt5_add_translation(QM_FILES ${TRANSLATIONS})

include_directories(${TOP_DIR}/3rdparty/kyplugin/)
include_directories(${TOP_DIR}/interface/)
include_directories(${TOP_DIR}/kom/)
include_directories(${TOP_DIR}/plugins/common/)
include_directories(${TOP_DIR}/plugins/fault_diagnosis/component/)
include_directories(${TOP_DIR}/plugins/fault_diagnosis/diagnoses/)
include_directories(${TOP_DIR}/plugins/junk_clean/cleaner/)
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(${PROJECT_NAME} SHARED ${SRCS} ${QM_FILES} ${COMMON} ${COMPONENT} ${DIAGNOSES} ${CLEANER})
target_link_libraries(${PROJECT_NAME} PRIVATE kom)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets Qt5::DBus Qt5::Xml)

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

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kylin-os-manager/plugins/)
install(FILES ${QM_FILES} DESTINATION /usr/share/kylin-os-manager/plugins/home_page/translations/)
