cmake_minimum_required(VERSION 3.16)
project(ernie-bot)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5
        COMPONENTS Core Gui Quick Widgets Network LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}
        COMPONENTS Core Gui Quick Widgets Network LinguistTools REQUIRED)

find_package(ukui-menu REQUIRED)

set(QRC_FILES "content/content.qrc")
# 翻译文件
file(GLOB TS_FILES "translations/*.ts")

# 更新翻译并创建.qm文件
if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${PROJECT_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${PROJECT_SOURCE_DIR} ${TS_FILES})
endif()

set(SOURCE
        src/ernie-bot.h src/ernie-bot.cpp
        src/chat.h src/chat.cpp
        src/message.cpp src/message.h
        src/message-model.cpp src/message-model.h
        src/message-action.cpp src/message-action.h
        src/session-manager.cpp src/session-manager.h
        src/session.cpp src/session.h
        )

add_library(ernie-bot SHARED ${SOURCE} ${QM_FILES} ${QRC_FILES})
target_link_libraries(ernie-bot  PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Network
        ukui-menu
        )

install(TARGETS ernie-bot LIBRARY DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/ukui-menu/extensions")
install(FILES   ${QM_FILES} DESTINATION "/usr/share/ukui-menu/extensions/translations/")
if (BUILD_TEST)
    add_subdirectory(tests)
endif ()
