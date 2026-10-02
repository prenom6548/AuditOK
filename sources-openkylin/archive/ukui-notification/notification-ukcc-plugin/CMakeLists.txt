
set(notification-ukcc-plugin_LIB_SRCS
        notification-ukcc-plugin.cpp
        notification-ukcc-plugin.h
        notice-menu.cpp
        notice-menu.h
        notification-settings-item.cpp
        notification-settings-item.h
)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
set(UKUI_NOTIFICATION_EXTERNAL_LIBS "")
set(UKUI_NOTIFICATION_PC_PKGS kysdk-qtwidgets)

foreach(PC_LIB IN ITEMS ${UKUI_NOTIFICATION_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND UKUI_NOTIFICATION_EXTERNAL_LIBS ${${PC_LIB}_LIBRARIES})
    endif()
endforeach()

include_directories(${Qt5Core_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS})
include_directories(../libukui-notification/notification-settings)
include_directories(../libukui-notification)

file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts)
# 更新翻译并创建.qm文件
if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
endif()

add_library(notification-ukcc-plugin SHARED
    ${notification-ukcc-plugin_LIB_SRCS}
    ${QM_FILES}
    )

target_link_libraries(notification-ukcc-plugin
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Network
        ukui-notification
        ukcc
        ${UKUI_NOTIFICATION_EXTERNAL_LIBS}
        )

install(TARGETS notification-ukcc-plugin DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/ukui-control-center)
install(FILES ${TS_FILES} ${QM_FILES} DESTINATION /usr/share/ukui-notification/notification-ukcc-plugin/translations)

