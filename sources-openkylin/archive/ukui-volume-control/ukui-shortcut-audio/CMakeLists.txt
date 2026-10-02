cmake_minimum_required(VERSION 3.14)

#定义项目名称和版本号
project(ukui-shortcut-audio)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(UKUI_SHORTCUT_AUDIO_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#查找Qt库
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Quick DBus LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Quick DBus LinguistTools REQUIRED)

include_directories(src/common/)
include_directories(src/control/)
include_directories(src/extra/)
include_directories(src/model/)
include_directories(src/utils/)
include_directories(config/)
include_directories(res/)
include_directories(translations/)
include_directories(qml/)
include_directories(../common)
include_directories(../backend)

#设置数据目录和翻译目录
set(UKUI_SHORTCUT_AUDIO_DATA_DIR "/usr/share/ukui/widgets/org.ukui.shortcut.audio")
set(UKUI_SHORTCUT_AUDIO_TRANSLATION_DIR "${UKUI_SHORTCUT_AUDIO_DATA_DIR}/translations")

set(SOURCE_FILES
    src/ukui-audio-main.cpp
    src/ukui-audio-main.h
    src/common/common.cpp
    src/common/common.h
    src/control/ukui-audio-control.cpp
    src/control/ukui-audio-control.h
    src/control/ukui-audio-sink-control.cpp
    src/control/ukui-audio-sink-control.h
    src/control/ukui-audio-source-control.cpp
    src/control/ukui-audio-source-control.h
    src/model/ukui-audio-sink-model.cpp
    src/model/ukui-audio-sink-model.h
    src/model/ukui-audio-sink-app-volume-model.cpp
    src/model/ukui-audio-sink-app-volume-model.h
    src/model/ukui-audio-source-model.cpp
    src/model/ukui-audio-source-model.h
    ../backend/Util.h
    src/extra/BaseType.h
    ../common/DBusClient.cpp
    ../common/DBusClient.h
    )

set(CONFIG_FILES
    config/qmldir
    )

set(QML_META_FILES
    qml/metadata.json
    )

set(QRC_FILES
    qml/qml.qrc
    res/res.qrc
    )

file(GLOB TS_FILES translations/*.ts)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    add_library(${PROJECT_NAME} SHARED MANUAL_FINALIZATION ${SOURCE_FILES} ${CONFIG_FILES} ${QML_META_FILES} ${QRC_FILES} ${QM_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CONFIG_FILES} ${QML_META_FILES} ${QRC_FILES} ${QM_FILES})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    )

target_include_directories(${PROJECT_NAME}
    PUBLIC ../backend ../common
    )

message("UKUI_SHORTCUT_AUDIO_DATA_DIR:" ${UKUI_SHORTCUT_AUDIO_DATA_DIR})
message("UKUI_SHORTCUT_AUDIO_TRANSLATION_DIR:" ${UKUI_SHORTCUT_AUDIO_TRANSLATION_DIR})

install(DIRECTORY "qml/" DESTINATION ${UKUI_SHORTCUT_AUDIO_DATA_DIR})
install(DIRECTORY "res/" DESTINATION "${UKUI_SHORTCUT_AUDIO_DATA_DIR}/res")
install(FILES ${QM_FILES} DESTINATION ${UKUI_SHORTCUT_AUDIO_TRANSLATION_DIR})
install(FILES ${CONFIG_FILES} DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/shortcut/audio")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION  "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/shortcut/audio")
