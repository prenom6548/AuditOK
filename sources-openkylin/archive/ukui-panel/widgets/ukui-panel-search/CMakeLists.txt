cmake_minimum_required(VERSION 3.16)

project(ukui-panel-search VERSION 4.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Widgets DBus LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Widgets DBus LinguistTools REQUIRED)
find_package(ukui-quick COMPONENTS platform)

set(PROJECT_SOURCES
        plugin/search-button.h
        plugin/search-plugin.h
        plugin/search-plugin.cpp
        plugin/search-button.cpp
)
file(GLOB TS_FILES translations/*.ts)
set(SEARCH_WIDGET_DIR "/usr/share/ukui/widgets/org.ukui.panel.search")
set(TRANSLATION_FILE_DIR "${SEARCH_WIDGET_DIR}/translations")
add_compile_definitions(TRANSLATION_FILE_DIR="${TRANSLATION_FILE_DIR}"
        UKUI_TASK_MANAGER_VERSION="${UKUI_TASK_MANAGER_VERSION}")

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}  ${TS_FILES})
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${QM_FILES})
else()
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${QM_FILES})
endif()

target_compile_definitions(${PROJECT_NAME}
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Widgets
        ukui-quick::platform
)

install(DIRECTORY "widget/" DESTINATION ${SEARCH_WIDGET_DIR})
install(FILES ${QM_FILES} DESTINATION "${TRANSLATION_FILE_DIR}")
install(FILES "plugin/qmldir" DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/panel/search")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION  "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/panel/search")
install(FILES ukui-panel-search-v11.json DESTINATION "/opt/system/conf/ukui-security-support/")
