# AUTOMOC gathers many moc_*.cpp into one mocs_compilation.cpp. Peony headers #undef Qt's
# signals/slots; restore macros after those includes in qt5-ukui-filedialog (kyfiledialog.h,
# pathbar.h, sidebar.h). CMake has no supported switch to split moc TUs for Qt5 here.

set(CMAKE_AUTOMOC on)
set(CMAKE_AUTOMOC on)
set(CMAKE_AUTOMOC on)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.66.0")

#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_CURRENT_SOURCE_DIR}/cmake/kde-modules ${CMAKE_MODULE_PATH})

# 查找Qt组件
find_package(QT NAMES Qt6 Qt5 COMPONENTS Gui Test DBus Qml Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Test DBus Qml Widgets REQUIRED)

# 查找PkgConfig工具
find_package(PkgConfig REQUIRED)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    DBus
    Widgets
    X11Extras
)

find_package(gsettings-qt)
find_package(X11)

include_directories(${Qt5Widgets_PRIVATE_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

find_package(KF5WindowSystem ${KF5_MIN_VERSION} CONFIG REQUIRED COMPONENTS)

find_package(KF5Wayland CONFIG REQUIRED COMPONENTS)
pkg_check_modules(Qsettings REQUIRED gsettings-qt)
pkg_check_modules(KYSDKCONF2 REQUIRED kysdk-conf2)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0 )
pkg_check_modules(PEONY REQUIRED peony)
find_package(Qt5LinguistTools)

find_package(KF5WindowSystem ${KF5_MIN_VERSION} CONFIG REQUIRED COMPONENTS)
find_package(KF5Wayland CONFIG REQUIRED COMPONENTS)

find_package(X11)

find_package(PkgConfig)
pkg_check_modules(QGSETTINGS REQUIRED gsettings-qt)
pkg_check_modules(PEONY REQUIRED peony)
pkg_check_modules(REQUIRED kysdk-waylandhelper)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0 )
pkg_check_modules(KYSDKWAYLANDHELPER REQUIRED kysdk-waylandhelper)
pkg_check_modules(KYSDKCONF2 REQUIRED kysdk-conf2)
pkg_check_modules(UKUIWINDOWHELPER REQUIRED kysdk-ukuiwindowhelper)
# KyFileOperationDialog : kdk::KDialog（file-operation-dialog）
pkg_check_modules(KYSDKQTWIDGETS REQUIRED kysdk-qtwidgets)

message(${imported_location})
set(LIB_PATH ${imported_location})
get_filename_component(PARENT_PATH "${LIB_PATH}" PATH)
get_filename_component(PARENT_PATH "${PARENT_PATH}" PATH)
message("qt5-ukui-filedialog PARENT_PATH: ${PARENT_PATH}")
set(TARGET_PATH ${PARENT_PATH})

include_directories("/usr/lib/x86_64-linux-gnu/qt5/plugins/platformthemes")
include_directories(${QGSETTINGS_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIRS})

if (PEONY_FOUND)
    include_directories(${PEONY_INCLUDE_DIRS})
    link_directories(${PEONY_LIBRARY_DIRS})
endif()
if (KYSDKWAYLANDHELPER_FOUND)
    include_directories(${KYSDKWAYLANDHELPER_INCLUDE_DIRS})
    link_directories(${KYSDKWAYLANDHELPER_LIBRARY_DIRS})
endif()
if (QGSETTINGS_FOUND)
    include_directories(${QGSETTINGS_INCLUDE_DIRS})
    link_directories(${QGSETTINGS_LIBRARY_DIRS})
endif()

if (KYSDKCONF2_FOUND)
    include_directories(${KYSDKCONF2_INCLUDE_DIRS})
    link_directories(${KYSDKCONF2_LIBRARY_DIRS})
endif()

if (UKUIWINDOWHELPER_FOUND)
    include_directories(${UKUIWINDOWHELPER_INCLUDE_DIRS})
    link_directories(${UKUIWINDOWHELPER_LIBRARY_DIRS})
endif()

if (KYSDKQTWIDGETS_FOUND)
    include_directories(${KYSDKQTWIDGETS_INCLUDE_DIRS})
    link_directories(${KYSDKQTWIDGETS_LIBRARY_DIRS})
endif()

include_directories(${Qsettings_INCLUDE_DIRS})
# 设置库文件名变量
include_directories(../../libqt5-ukui-style)
include_directories(../../qt5-ukui-filedialog)
include_directories(../../qt5-ukui-filedialog/file-operation-dialog)
include_directories(../../qt5-ukui-platformtheme/widget/filedialog)
include_directories(../../ukui-styles)

# 将库文件路径添加到库搜索路径中
link_directories(${LIBRARY_DIR})

file(GLOB_RECURSE test_settings_sources
    test-settings.cpp
    ../../libqt5-ukui-style/effects/highlight-effect.h
    ../../libqt5-ukui-style/effects/highlight-effect.cpp
    ../../libqt5-ukui-style/settings/application-style-settings.h
    ../../libqt5-ukui-style/settings/application-style-settings.cpp
    ../../libqt5-ukui-style/settings/ukui-style-conf-settings.h
    ../../libqt5-ukui-style/settings/ukui-style-conf-settings.cpp
    ../../libqt5-ukui-style/settings/ukui-style-settings.h
    ../../libqt5-ukui-style/settings/ukui-style-settings.cpp
    ../../libqt5-ukui-style/settings/black-list.h

    # 新增：InternalStyle 和 MPSStyle
    ../../libqt5-ukui-style/internal-styles/internal-style.h
    ../../libqt5-ukui-style/internal-styles/internal-style.cpp
    ../../libqt5-ukui-style/internal-styles/mps-style.h
    ../../libqt5-ukui-style/internal-styles/mps-style.cpp

    # 新增：AnimationHelper 和 AnimatorIface
    ../../libqt5-ukui-style/animations/animation-helper.h
    ../../libqt5-ukui-style/animations/animator-iface.h
    ../../libqt5-ukui-style/animations/animator-plugin-iface.h

    ../../libqt5-ukui-style/animations/scrollbar/ukui-scrollbar-default-interaction-animator.h
    ../../libqt5-ukui-style/animations/scrollbar/ukui-scrollbar-default-interaction-animator.cpp
    ../../libqt5-ukui-style/animations/tabwidget/ukui-tabwidget-animator-iface.h
    ../../libqt5-ukui-style/animations/tabwidget/ukui-tabwidget-animator-plugin-iface.h
    ../../libqt5-ukui-style/animations/tabwidget/ukui-tabwidget-default-slide-animator-factory.h
    ../../libqt5-ukui-style/animations/tabwidget/ukui-tabwidget-default-slide-animator.h
    ../../libqt5-ukui-style/animations/tabwidget/ukui-tabwidget-default-slide-animator.cpp
    ../../libqt5-ukui-style/animations/tabwidget/ukui-tabwidget-default-slide-animator-factory.cpp
    ../../libqt5-ukui-style/gestures/ukui-two-finger-slide-gesture.h
    ../../libqt5-ukui-style/gestures/ukui-two-finger-zoom-gesture.h
    ../../libqt5-ukui-style/gestures/ukui-two-finger-slide-gesture.cpp
    ../../libqt5-ukui-style/gestures/ukui-two-finger-zoom-gesture.cpp
    ../../ukui-styles/qt5-config-style-ukui/ukui-config-style-parameter.cpp
    ../../ukui-styles/qt5-config-style-ukui/ukui-config-style-parameter.h
    # 移除通配符包含，使用显式包含来避免 moc 文件重复
    ../../ukui-styles/blur-helper.h
    ../../ukui-styles/blur-helper.cpp
    ../../ukui-styles/gesture-helper.h
    ../../ukui-styles/gesture-helper.cpp
    ../../ukui-styles/window-manager.h
    ../../ukui-styles/window-manager.cpp
    # 移除 ukui-style-plugin.h/cpp 因为 Q_PLUGIN_METADATA 宏会导致重复定义
    # ../../ukui-styles/ukui-style-plugin.h
    # ../../ukui-styles/ukui-style-plugin.cpp
    ../../ukui-styles/readconfig.h
    ../../ukui-styles/readconfig.cpp

    # 添加 ukui-config-style.cpp
    ../../ukui-styles/qt5-config-style-ukui/ukui-config-style.cpp
    ../../ukui-styles/qt5-config-style-ukui/ukui-config-style.h
    ../../ukui-styles/qt5-config-style-ukui/config-shadow-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/config-shadow-helper.h
    ../../ukui-styles/qt5-config-style-ukui/config-style-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/config-style-helper.h
    ../../ukui-styles/qt5-config-style-ukui/config-view-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/config-view-helper.h
    ../../ukui-styles/qt5-config-style-ukui/ukui-config-style-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/ukui-config-style-parameters.h

    # 添加 animations 目录下的 cpp 文件
    ../../ukui-styles/qt5-config-style-ukui/animations/config-box-animation-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-box-animator.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-button-animation-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-button-animator.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-checkbox-animation-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-checkbox-animator.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-progressbar-animation.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-progressbar-animation-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-radiobutton-animation-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-radiobutton-animator.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-scrollbar-animation-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-slider-animation-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-slider-animator.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-tab-widget-animation-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-tree-animation-helper.cpp
    ../../ukui-styles/qt5-config-style-ukui/animations/config-tree-animator.cpp

    # 添加 widget-parameters 目录下的 cpp 文件
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/control-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-checkbox-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-combobox-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-lineedit-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-list-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-menu-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-progressbar-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-pushbutton-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-radiobutton-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-scrollbar-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-slider-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-spinbox-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-table-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-tabwidget-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-toolbutton-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-tooltip-parameters.cpp
    ../../ukui-styles/qt5-config-style-ukui/widget-parameters/config-tree-parameters.cpp

    ../../qt5-ukui-filedialog/pathbar.h
    ../../qt5-ukui-filedialog/sidebar.h
    ../../qt5-ukui-filedialog/debug.h
    ../../qt5-ukui-filedialog/kyfiledialog.h
    ../../qt5-ukui-filedialog/uikyfiledialog.h
    ../../qt5-ukui-filedialog/filedialoginterface.h
    ../../qt5-ukui-filedialog/filedialogplugin_global.h
    ../../qt5-ukui-filedialog/filedialogplugin.h
    ../../qt5-ukui-filedialog/kyfiledialog.h
    ../../qt5-ukui-filedialog/kyfiledialogprivate.h
    ../../qt5-ukui-filedialog/menutoolbutoon.h
    ../../qt5-ukui-filedialog/uikyfiledialog.h

    ../../qt5-ukui-filedialog/filedialogplugin.cpp
    ../../qt5-ukui-filedialog/kyfiledialog.cpp
    ../../qt5-ukui-filedialog/kyfiledialogprivate.cpp
    ../../qt5-ukui-filedialog/menutoolbutoon.cpp
    ../../qt5-ukui-filedialog/pathbar.cpp
    ../../qt5-ukui-filedialog/sidebar.cpp
    ../../qt5-ukui-filedialog/uikyfiledialog.cpp
    ../../qt5-ukui-filedialog/file-operation-dialog/kyfiledialogrename.cpp
    ../../qt5-ukui-filedialog/file-operation-dialog/kyfiledialogrename.h
    ../../qt5-ukui-filedialog/file-operation-dialog/kyfileoperationdialog.cpp
    ../../qt5-ukui-filedialog/file-operation-dialog/kyfileoperationdialog.h
    ../../qt5-ukui-filedialog/file-operation-dialog/rename-editor.cpp
    ../../qt5-ukui-filedialog/file-operation-dialog/rename-editor.h

    ../../qt5-ukui-platformtheme/widget/filedialog/xdgdesktopportalfiledialog.cpp
)

add_executable(test-settings ${test-settings_SRCS} ${test_settings_sources} )

set(PLUGIN_INSTALL_FILES /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/peony-extensions/libpeony-computer-view-plugin.so)
set(FILEBOX_PLUGIN_INSTALL_FILES /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/peony-extensions)
target_compile_definitions(test-settings PRIVATE
    PLUGIN_INSTALL_FILES="${PLUGIN_INSTALL_FILES}"
    FILEBOX_PLUGIN_INSTALL_FILES="${FILEBOX_PLUGIN_INSTALL_FILES}")

file(GLOB_RECURSE test_settings_headers ../../ukui-styles/**/*.h)

set(test-settings_SRCS ${test_settings_headers})

target_link_libraries(test-settings PRIVATE Qt${QT_VERSION_MAJOR}::Test
    Qt5::Widgets
    Qt5::DBus
    Qt5::Gui
    Qt5::X11Extras
    KF5::WaylandClient
    KF5::WindowSystem
    ${QGSETTINGS_LIBRARIES}
    ${KYSDKWAYLANDHELPER_LIBRARIES}
    xcb
    glib-2.0
    ${PEONY_LIBRARIES}
    ${XCB_LIBS}
    gsettings-qt
    qt5-ukui-style
    ${KYSDKCONF2_LIBRARIES}
    ${UKUIWINDOWHELPER_LIBRARIES}
    ${KYSDKQTWIDGETS_LIBRARIES})

set(test_libs
    Qt5::Widgets
    glib-2.0
    peony
    )

SET(_code "
    #include <glib-object.h>
    #include <peony-qt/search-vfs-uri-parser.h>

    int main()
    {
       Peony::SearchVFSUriParser::addSearchKey(\"\", true);
       return 0;
    }")
SET(_file ${CMAKE_CURRENT_BINARY_DIR}/DetermineHavePeonyInterface.cpp)
FILE(WRITE "${_file}" "${_code}")
TRY_COMPILE(HAVE_PEONY_INTERFACE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${_file}"
    LINK_LIBRARIES ${test_libs}
    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${GLIB2_INCLUDE_DIRS} ${PEONY_INCLUDE_DIRS}"
    OUTPUT_VARIABLE OUTPUT)
MESSAGE("Build output: ${OUTPUT} HAVE_PEONY_INTERFACE: ${HAVE_PEONY_INTERFACE}")

if(HAVE_PEONY_INTERFACE)
    add_definitions(-DHAVE_PEONY_INTERFACE=1)
endif()

# 添加测试用例
add_test(NAME test-settings COMMAND test-settings)
add_definitions(-DQT5UKUIPLATFORMTHEME_LIBRARY)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_MESSAGELOGCONTEXT)
