cmake_minimum_required(VERSION 3.16)
project(ukui-sidebar)
set(VERSION "3.22.0")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# see https://cmake.org/cmake/help/v3.16/manual/cmake-qt.7.html
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# see: https://cmake.org/cmake/help/v3.16/command/add_subdirectory.html binary_dir
add_subdirectory(3rd-parties/qtsingleapplication)
add_subdirectory(ukui-shortcut)
add_subdirectory(ukui-sidebar-items)

# 查找qt组件
find_package(QT NAMES Qt6 Qt5 COMPONENTS
        Core Gui Quick Widgets LinguistTools DBus Quick Qml
        REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
        Core Gui Quick Widgets LinguistTools DBus Quick Qml
        REQUIRED)

if (QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 COMPONENTS X11Extras REQUIRED)
endif()

# find kde framework components
# see: https://api.kde.org/frameworks/index.html > KWindowSystem
if (QT_VERSION_MAJOR EQUAL 6)
    find_package(KF6WindowSystem REQUIRED)
elseif (QT_VERSION_MAJOR EQUAL 5)
    find_package(KF5WindowSystem REQUIRED)
endif()

find_package(ukui-notification REQUIRED)
find_package(ukui-quick COMPONENTS core platform framework REQUIRED)

# 查找其他组件
# see: https://cmake.org/cmake/help/v3.16/module/FindPkgConfig.html
# see: https://cmake.org/cmake/help/v3.16/command/list.html
find_package(PkgConfig REQUIRED)

set(UKUI_SIDEBAR_EXTERNAL_LIBS "")
if (QT_VERSION_MAJOR EQUAL "5")
    set(UKUI_SIDEBAR_PC_PKGS gsettings-qt x11 xcb kysdk-datacollect gio-2.0 kysdk-systime)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(UKUI_SIDEBAR_PC_PKGS gsettings-qt6 x11 xcb kysdk-datacollect gio-2.0 kysdk-systime)
endif()

foreach(PC_LIB IN ITEMS ${UKUI_SIDEBAR_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND UKUI_SIDEBAR_EXTERNAL_LIBS ${${PC_LIB}_LIBRARIES})
    endif()
endforeach()

message(STATUS "External libraries found: ${UKUI_SIDEBAR_EXTERNAL_LIBS}")

# include文件夹
include_directories(3rd-parties/qtsingleapplication/src)
include_directories(src)
include_directories(src/main)
include_directories(src/items)
include_directories(src/model)
include_directories(src/config)
include_directories(src/utils)
include_directories(src/windows)
include_directories(src/notification)
include_directories(src/notification/notification-helper)
# 插件目录
include_directories(ukui-shortcut)
include_directories(ukui-shortcut/interface)

# 用于Qt Creator识别自定义qml模块的导入路径
list(APPEND QML_MODULE_DIRS "qml")
set(QML_IMPORT_PATH "${QML_MODULE_DIRS}" CACHE STRING "Qt Creator extra qml import paths." FORCE)
message(STATUS "QML_IMPORT_PATH: ${QML_IMPORT_PATH}")

# 基础设置
set(TRANSLATION_FILE_DIR "/usr/share/ukui-sidebar/translations")

# 宏定义
add_compile_definitions(TRANSLATION_FILE_DIR="${TRANSLATION_FILE_DIR}"
        VERSION="${VERSION}")

# 源码
set(SOURCE_FILES
        src/main.cpp
        src/main/app-manager.h src/main/app-manager.cpp
        src/main/screen-monitor.h src/main/screen-monitor.cpp
        src/main/global-settings.h src/main/global-settings.cpp
        src/main/hand-gesture-helper.h src/main/hand-gesture-helper.cpp
        src/main/account-information.h src/main/account-information.cpp
        src/sidebar-main.h src/sidebar-main.cpp
        src/sidebar-dbus-service.h src/sidebar-dbus-service.cpp
        src/utils/date-time-utils.h src/utils/date-time-utils.cpp
        src/utils/event-track.cpp src/utils/event-track.h
        src/model/shortcut-model.h src/model/shortcut-model.cpp
        src/model/edit-shortcut-model.h src/model/edit-shortcut-model.cpp
        src/model/shortcut-filter-model.h src/model/shortcut-filter-model.cpp
        src/model/widget-delegate.h src/model/widget-delegate.cpp
        src/model/shortcut-config.h src/model/shortcut-config.cpp
        src/model/shortcut-item.h src/model/shortcut-item.cpp
        src/model/plugin-state.h
        src/windows/window-type.h
        src/windows/right-hand-gesture-view.cpp src/windows/right-hand-gesture-view.h
        src/windows/sidebar-window-helper.h src/windows/sidebar-window-helper.cpp
        src/windows/window-blur-helper.h src/windows/window-blur-helper.cpp
        src/windows/sidebar-view.cpp src/windows/sidebar-view.h
        src/windows/shortcuts-window.cpp src/windows/shortcuts-window.h
        src/log-utils.cpp src/log-utils.h
)

set(NOTIFICATION_SOURCE_FILES
        src/notification/notification-model.cpp src/notification/notification-model.h
        src/notification/notification-group-model.cpp src/notification/notification-group-model.h
        src/notification/notification-utils.cpp src/notification/notification-utils.h
        src/notification/urgency-notification-model.cpp src/notification/urgency-notification-model.h
        src/notification/popup-notification-model.cpp src/notification/popup-notification-model.h
        src/windows/notification-window.cpp src/windows/notification-window.h
        src/windows/popup-notification-window.cpp src/windows/popup-notification-window.h
        src/windows/tablet-popup-view.cpp src/windows/tablet-popup-view.h
        src/notification/group-cache-proxy-model.cpp src/notification/group-cache-proxy-model.h
        src/windows/status-bar-view.cpp src/windows/status-bar-view.h
        src/layout/ukui-column.cpp src/layout/ukui-column.h
)

if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(SOURCE_FILES data/org.ukui.sidebar.xml sidebar-dbus-service.h Sidebar::SidebarDbusService)
else()
    qt5_add_dbus_adaptor(SOURCE_FILES data/org.ukui.sidebar.xml sidebar-dbus-service.h Sidebar::SidebarDbusService)
endif()

# qrc文件
set(QRC_FILES qml/qml.qrc res/res.qrc)
# desktop file
set(DESKTOP_FILE data/ukui-sidebar.desktop)

# 翻译文件
file(GLOB TS_FILES "translations/*.ts")

# 更新翻译并创建.qm文件
if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/qml ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/qml ${TS_FILES})
endif()

add_executable(
        ${PROJECT_NAME}
        ${QRC_FILES}
        ${QM_FILES}
        ${SOURCE_FILES}
        ${NOTIFICATION_SOURCE_FILES}
)
target_include_directories(${PROJECT_NAME} PRIVATE ${Qt5Quick_PRIVATE_INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:UKUI_SIDEBAR_LOG_FILE_DISABLE>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::QmlPrivate
        Qt${QT_VERSION_MAJOR}::QuickPrivate
        KF${QT_VERSION_MAJOR}::WindowSystem
        qtsingleapplication
        ukui-shortcut
        ukui-notification
        ${UKUI_SIDEBAR_EXTERNAL_LIBS}
        ukui-quick::core
        ukui-quick::platform
        ukui-quick::framework
        Qt${QT_VERSION_MAJOR}::Qml
        )

if (QT_VERSION_MAJOR EQUAL 5)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Qt5::X11Extras
    )
elseif(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Qt::GuiPrivate
    )
endif()

# 安装ukui-menu
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "/usr/bin")
# 安装翻译文件
install(FILES ${QM_FILES} DESTINATION "${TRANSLATION_FILE_DIR}")
# 安装desktop文件
install(FILES ${DESKTOP_FILE} DESTINATION "/etc/xdg/autostart")

if (BUILD_TEST)
    enable_testing()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    add_subdirectory(autotests)
endif ()