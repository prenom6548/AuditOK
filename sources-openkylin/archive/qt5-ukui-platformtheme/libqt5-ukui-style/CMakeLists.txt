cmake_minimum_required(VERSION 3.16)

# 禁用编译器测试
set(CMAKE_TRY_COMPILE_COMPILER_FLAG_FIRST TRUE)
set(CMAKE_CXX_IGNORE_EXTRA_INCLUDES ON)

project(qt5-ukui-style)

# 设置共享库的版本号
set(QT5_UKUI-STYLE_VERSION_MAJOR 1)
set(QT5_UKUI-STYLE_VERSION_MINOR 0)
set(QT5_UKUI-STYLE_VERSION_PATCH 0)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.66.0")

find_package(Qt5Concurrent)
set(LIB_PATH ${imported_location})
get_filename_component(PARENT_PATH "${LIB_PATH}" PATH)
message("libqt5-ukui-style PARENT_PATH: ${PARENT_PATH}")

find_package(Qt5Widgets)

include_directories(${Qt5Widgets_PRIVATE_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

#find_package(Qt5QtThemeSupport REQUIRED)

find_package(X11)

find_package(PkgConfig)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0 )
pkg_check_modules(Qsettings REQUIRED gsettings-qt)
pkg_check_modules(KYSDKCONF2 REQUIRED kysdk-conf2)

include_directories(${Qsettings_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIRS})
if (KYSDKCONF2_FOUND)
    include_directories(${KYSDKCONF2_INCLUDE_DIRS})
    link_directories(${KYSDKCONF2_LIBRARY_DIRS})
endif()

file(GLOB_RECURSE Header "*.h" "*.hpp")
file(GLOB_RECURSE Sources "*.cpp" "*.c" "*.ui")
file(GLOB_RECURSE XML "org.ukui.style.gschema.xml")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Header} ${Sources} ${XML})

add_library(qt5-ukui-style SHARED ${Sources} ${Header} ${XML})

# 设置共享库的命名规则
set_target_properties(qt5-ukui-style PROPERTIES
    OUTPUT_NAME "qt5-ukui-style"
    VERSION ${QT5_UKUI-STYLE_VERSION_MAJOR}.${QT5_UKUI-STYLE_VERSION_MINOR}.${QT5_UKUI-STYLE_VERSION_PATCH}
    SOVERSION ${QT5_UKUI-STYLE_VERSION_MAJOR}
)

# 允许链接时的多重定义
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-multiple-definition")

target_link_libraries(qt5-ukui-style PRIVATE
    Qt5::Widgets
    Qt5::Concurrent
    gsettings-qt
    gio-2.0
    ${KYSDKCONF2_LIBRARIES}
    )


add_definitions(-DLIBQT5UKUISTYLE_LIBRARY)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_MESSAGELOGCONTEXT)


if(UNIX)
    set(TARGET_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")

    install(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PATH})

    set(GSCHEMA_PATH "/usr/share/glib-2.0/schemas")
    set(GSCHEMA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/settings/org.ukui.style.gschema.xml")
    install(FILES ${GSCHEMA_FILE} DESTINATION ${GSCHEMA_PATH})

    set(KCONF2_PATH "/etc/kylin-config/basic/")
    set(KCONF2_FILE "${CMAKE_CURRENT_SOURCE_DIR}/settings/org.ukui.style.yaml")
    install(FILES ${KCONF2_FILE} DESTINATION ${KCONF2_PATH})

    set(PKGCONFIG_PATH "${TARGET_PATH}/pkgconfig")
    set(PKGCONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/development-files/qt5-ukui.pc")
    install(FILES ${PKGCONFIG_FILE} DESTINATION ${PKGCONFIG_PATH})

    install(FILES ${Header} DESTINATION "libqt5-ukui-style/")

endif()
