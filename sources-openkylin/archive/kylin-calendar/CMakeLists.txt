cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(calendar)

include(GNUInstallDirs)

# 基础配置
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PLUGIN "calendar")

# 依赖版本配置
set(KF5_MINIMUM_VERSION "5.18.0")
set(QTXDG_MINIMUM_VERSION "3.3.1")

# 查找依赖包
find_package(KF5WindowSystem ${KF5_MINIMUM_VERSION} REQUIRED)
find_package(Qt5Xdg ${QTXDG_MINIMUM_VERSION} REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Sql Svg Network DBus LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)

# 包含目录配置
include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    /path/to/kdk
)

# 查找ukui-log4qt库
find_library(UKUI_LOG4QT_LIB ukui-log4qt)
if(NOT UKUI_LOG4QT_LIB)
    message(FATAL_ERROR "ukui-log4qt library not found!")
endif()

# pkg-config依赖项配置
pkg_check_modules(KYSDK_DATACOLLECT REQUIRED kysdk-datacollect)
include_directories(${KYSDK_DATACOLLECT_INCLUDE_DIRS})

pkg_check_modules(KYSDKUKUIWINDOWHELPER_PKG REQUIRED kysdk-ukuiwindowhelper)
include_directories(${KYSDKUKUIWINDOWHELPER_PKG_INCLUDE_DIRS})
link_directories(${KYSDKUKUIWINDOWHELPER_PKG_LIBRARY_DIRS})

pkg_check_modules(Gsetting REQUIRED gsettings-qt)
include_directories(${Gsetting_INCLUDE_DIRS})
link_directories(${Gsetting_PKG_LIBRARY_DIRS})
link_libraries(gsettings-qt.so)

pkg_check_modules(CALENDAR_DEPS REQUIRED glib-2.0)
link_libraries(glib-2.0.so)

#pkg_check_modules(CALENDAR_DEPS REQUIRED libqt5dbus5)
#link_libraries(libQt5DBus.so.5)

pkg_check_modules(KYSDKSYSTIME_PKG kysdk-systime)
include_directories(${KYSDKSYSTIME_PKG_INCLUDE_DIRS})
link_directories(${KYSDKSYSTIME_PKG_LIBRARY_DIRS})
link_libraries(kydate.so)

pkg_check_modules(KYSDKQTWIDGETS_PKG kysdk-qtwidgets)
include_directories(${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS})
link_directories(${KYSDKQTWIDGETS_PKG_LIBRARY_DIRS})
link_libraries(kysdk-qtwidgets.so)

pkg_check_modules(KYSDKKABASE_PKG kysdk-kabase)
include_directories(${KYSDKKABASE_PKG_INCLUDE_DIRS})
link_directories(${KYSDKKABASE_PKG_LIBRARY_DIRS})
link_libraries(kysdk-kabase.so)

pkg_check_modules(KYSDKWAYLANDHELPER_PKG kysdk-waylandhelper)
include_directories(${KYSDKWAYLANDHELPER_PKG_INCLUDE_DIRS})
link_directories(${KYSDKWAYLANDHELPER_PKG_LIBRARY_DIRS})
link_libraries(kysdk-waylandhelper.so)

message("${KYSDKWAYLANDHELPER_PKG_LIBRARY_DIRS}")
include_directories(${CALENDAR_DEPS_INCLUDE_DIRS})

set(HEADERS
    lunarcalendarwidget/frmlunarcalendarwidget.h
    lunarcalendarwidget/lunarcalendarinfo.h
    lunarcalendarwidget/lunarcalendaritem.h
    lunarcalendarwidget/lunarcalendaryearitem.h
    lunarcalendarwidget/lunarcalendarmonthitem.h
    lunarcalendarwidget/lunarcalendarwidget.h
    lunarcalendarwidget/calendarcolor.h
    lunarcalendarwidget/schedulewidget.h
    lunarcalendarwidget/calendardatabase.h
    lunarcalendarwidget/calendarinfo.h
    lunarcalendarwidget/schedulestruct.h
    lunarcalendarwidget/colorarea.h
    lunarcalendarwidget/schedule_item.h
    lunarcalendarwidget/custommessagebox.h
    lunarcalendarwidget/notificationsadaptor.h
    lunarcalendarwidget/customscrollarea.h
)

# 源文件列表
set(SOURCES
    lunarcalendarwidget/frmlunarcalendarwidget.cpp
    lunarcalendarwidget/lunarcalendarinfo.cpp
    lunarcalendarwidget/lunarcalendaritem.cpp
    lunarcalendarwidget/lunarcalendaryearitem.cpp
    lunarcalendarwidget/lunarcalendarmonthitem.cpp
    lunarcalendarwidget/lunarcalendarwidget.cpp
    lunarcalendarwidget/calendarcolor.cpp
    lunarcalendarwidget/schedulewidget.cpp
    lunarcalendarwidget/calendardatabase.cpp
    lunarcalendarwidget/calendarinfo.cpp
    lunarcalendarwidget/colorarea.cpp
    lunarcalendarwidget/schedule_item.cpp
    lunarcalendarwidget/custommessagebox.cpp
    lunarcalendarwidget/main.cpp
    lunarcalendarwidget/notificationsadaptor.cpp
    lunarcalendarwidget/customscrollarea.cpp
)

# UI文件列表
set(UIS
    lunarcalendarwidget/frmlunarcalendarwidget.ui
)

# 创建可执行文件
add_executable(kylin-calendar ${SOURCES} ${HEADERS} ${UIS})

# 链接库配置
target_link_libraries(kylin-calendar
    Qt5::Widgets
    Qt5::Svg
    KF5::WindowSystem
    ${Gsetting_LIBRARIES}
    Qt5::Sql
    Qt5::Core
    Qt5::DBus
    Qt5::Network
    pthread
    ${UKUI_LOG4QT_LIB}
    ${KYSDK_DATACOLLECT_LIBRARIES}
    kysdk-ukuiwindowhelper
)

# 资源文件配置
file(GLOB_RECURSE DESKTOP_FILE "resources/calendar.desktop")
set(PACKAGE_DATA_DIR "/usr/share/calendar")

file(GLOB_RECURSE NOTIFITY_FILE "resources/kylin-calendar.json")  # 通知配置文件
set(NOTIFITY_DATA_DIR "/etc/ukui/ukui-notification/default-app/")

file(GLOB_RECURSE GSETTING_SCHEMAS "resources/org.kylin.calendar.plugin.gschema.xml")
set(SCHEMAS_INSTALL_DIR "/usr/share/glib-2.0/schemas/")

# 安装配置
install(TARGETS kylin-calendar DESTINATION /usr/bin)
install(FILES ${DESKTOP_FILE} DESTINATION "/etc/xdg/autostart")
install(FILES ${GSETTING_SCHEMAS} DESTINATION ${SCHEMAS_INSTALL_DIR})
install(FILES ${NOTIFITY_FILE} DESTINATION ${NOTIFITY_DATA_DIR})  # 新增通知文件安装
install(DIRECTORY html/ DESTINATION ${PACKAGE_DATA_DIR}/plugin-calendar/html)

# 国际化配置
file(GLOB TS_FILES "translation/*.ts")

# 加载翻译文件
file(GLOB TS_FILES "${PROJECT_SOURCE_DIR}/translation/*.ts" )
# 更新翻译文件并创建.qm文件
qt5_create_translation(QM_FILES ${PROJECT_SOURCE_DIR} ${TS_FILES})
add_custom_target(translations ALL DEPENDS ${QM_FILES})
set(${PLUGIN}_QM_FILES ${QM_FILES})

# 翻译文件安装路径
set(CALENDAR_TRANSLATION_DIR "${PACKAGE_DATA_DIR}/plugin-calendar/translation")
add_compile_definitions(CALENDAR_TRANSLATION_DIR="${CALENDAR_TRANSLATION_DIR}")
install(FILES ${QM_FILES} DESTINATION ${CALENDAR_TRANSLATION_DIR})
