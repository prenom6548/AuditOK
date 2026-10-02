cmake_minimum_required(VERSION 3.14)

project(ukui-system-tray)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(UKUI_SYSTEM_TRAY_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Quick LinguistTools DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Quick LinguistTools DBus REQUIRED)
find_package(ukui-quick COMPONENTS framework platform)
find_package(ukui-search REQUIRED)

set(PROJECT_SOURCES
        plugin/system-tray-plugin.h plugin/system-tray-plugin.cpp
        plugin/tray-items-model.h plugin/tray-items-model.cpp
        plugin/tray-item.h plugin/tray-item.cpp
        plugin/item-group-model.h plugin/item-group-model.cpp
        plugin/security-function-control.h plugin/security-function-control.cpp
        plugin/items-filter-model.h plugin/items-filter-model.cpp
        plugin/control-panel-model.cpp
        plugin/control-panel-model.h
)

include_directories(${CMAKE_SOURCE_DIR}/status-notifier-host)

file(GLOB TS_FILES translations/*.ts)
set(UKUI_SYSTEM_TRAY_TRANSLATION_DIR "/usr/share/ukui/widgets/org.ukui.systemTray/translations")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${QRC_FILES} ${QM_FILES})

else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${QRC_FILES} ${QM_FILES})
endif()

target_compile_definitions(${PROJECT_NAME}
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        ukui-sni-host
        ukui-quick::framework
        ukui-quick::platform
        ukui-search
)

install(DIRECTORY "widget/" DESTINATION "/usr/share/ukui/widgets/org.ukui.systemTray")
install(FILES "plugin/qmldir" DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/qml/org/ukui/systemTray")
install(FILES ${QM_FILES} DESTINATION ${UKUI_SYSTEM_TRAY_TRANSLATION_DIR})
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION  "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/qml/org/ukui/systemTray")
install(FILES ukui-sni-v11.json DESTINATION "/opt/system/conf/ukui-security-support/")

if(BUILD_TEST)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    enable_testing()
    add_subdirectory(autotests)
endif()
