cmake_minimum_required(VERSION 3.16)

project(ukui-ai-assistant VERSION 4.20)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# TODO: 将依赖倒入的操作放到顶级CMakeList文件中
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick LinguistTools REQUIRED)
find_package(ukui-quick COMPONENTS platform REQUIRED)
find_package(ukui-search)

set(PROJECT_SOURCES
        plugin/ai-assistant-button.cpp
        plugin/ai-assistant-button.h
        plugin/ai-assistant-plugin.cpp
        plugin/ai-assistant-plugin.h
)

file(GLOB TS_FILES i18n/*.ts)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${QM_FILES})

else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${QM_FILES})
endif()

target_compile_definitions(${PROJECT_NAME}
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        ukui-quick::platform
        ukui-search
)

install(DIRECTORY "widget/" DESTINATION "/usr/share/ukui/widgets/org.ukui.ai.assistant")
install(FILES ${QM_FILES} DESTINATION "/usr/share/ukui/widgets/org.ukui.ai.assistant/i18n")
install(FILES "plugin/qmldir" DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/ai/assistant")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION  "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/ai/assistant")
install(FILES ukui-ai-assistant-v11.json DESTINATION "/opt/system/conf/ukui-security-support/")
