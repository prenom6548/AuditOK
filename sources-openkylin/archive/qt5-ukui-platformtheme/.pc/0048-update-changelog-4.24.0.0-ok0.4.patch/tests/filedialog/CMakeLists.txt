cmake_minimum_required(VERSION 3.16)

project(test_filedialog)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Define FILEBOX_PLUGIN_INSTALL_FILES for uikyfiledialog.cpp
set(FILEBOX_PLUGIN_INSTALL_FILES "/usr/lib/aarch64-linux-gnu/peony-extensions")

set(QT_MIN_VERSION "5.12.0")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Test
    DBus
    Concurrent
)

# Find required packages for KyFileDialog testing
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSETTINGS REQUIRED gsettings-qt)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)
pkg_check_modules(PEONY REQUIRED peony)
pkg_check_modules(UKUIWINDOWHELPER REQUIRED kysdk-ukuiwindowhelper)
pkg_check_modules(KYSDK_QTWIDGETS REQUIRED kysdk-qtwidgets)

include_directories(
    ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${GLIB2_INCLUDE_DIRS}
    ${PEONY_INCLUDE_DIRS}
    ${QGSETTINGS_INCLUDE_DIRS}
    ${UKUIWINDOWHELPER_INCLUDE_DIRS}
    ${KYSDK_QTWIDGETS_INCLUDE_DIRS}
    ../../qt5-ukui-filedialog
    ../../libqt5-ukui-style
)

link_directories(${PEONY_LIBRARY_DIRS})

# Include source files from qt5-ukui-filedialog that are needed for testing
set(FILEDIALOG_SOURCES
    ../../qt5-ukui-filedialog/kyfiledialog.cpp
    ../../qt5-ukui-filedialog/kyfiledialogprivate.cpp
    ../../qt5-ukui-filedialog/uikyfiledialog.cpp
    ../../qt5-ukui-filedialog/pathbar.cpp
    ../../qt5-ukui-filedialog/sidebar.cpp
    ../../qt5-ukui-filedialog/menutoolbutoon.cpp
    ../../qt5-ukui-filedialog/filedialogplugin.cpp
    ../../qt5-ukui-filedialog/file-operation-dialog/kyfiledialogrename.cpp
    ../../qt5-ukui-filedialog/file-operation-dialog/kyfileoperationdialog.cpp
    ../../qt5-ukui-filedialog/file-operation-dialog/rename-editor.cpp
)

# Include source files from libqt5-ukui-style that are needed for testing
set(UKUI_STYLE_SOURCES
    ../../libqt5-ukui-style/settings/ukui-style-settings.cpp
)

# Test 1: test_filedialog
set(TEST_FILEDIALOG_SOURCES
    test_filedialog.cpp
    ${FILEDIALOG_SOURCES}
    ${UKUI_STYLE_SOURCES}
)

add_executable(test_filedialog ${TEST_FILEDIALOG_SOURCES})

target_compile_definitions(test_filedialog PRIVATE
    FILEBOX_PLUGIN_INSTALL_FILES="${FILEBOX_PLUGIN_INSTALL_FILES}"
)

target_link_libraries(test_filedialog PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Test
    Qt5::DBus
    Qt5::Concurrent
    ${QGSETTINGS_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${PEONY_LIBRARIES}
    ${UKUIWINDOWHELPER_LIBRARIES}
    ${KYSDK_QTWIDGETS_LIBRARIES}
    glib-2.0
    peony
)

add_test(NAME test_filedialog COMMAND test_filedialog)

# Test 2: test_kyfiledialog_signals
set(TEST_KYFILEDIALOG_SIGNALS_SOURCES
    test_kyfiledialog_signals.cpp
    ${FILEDIALOG_SOURCES}
    ${UKUI_STYLE_SOURCES}
)

add_executable(test_kyfiledialog_signals ${TEST_KYFILEDIALOG_SIGNALS_SOURCES})

target_compile_definitions(test_kyfiledialog_signals PRIVATE
    FILEBOX_PLUGIN_INSTALL_FILES="${FILEBOX_PLUGIN_INSTALL_FILES}"
)

target_link_libraries(test_kyfiledialog_signals PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Test
    Qt5::DBus
    Qt5::Concurrent
    ${QGSETTINGS_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${PEONY_LIBRARIES}
    ${UKUIWINDOWHELPER_LIBRARIES}
    ${KYSDK_QTWIDGETS_LIBRARIES}
    glib-2.0
    peony
)

add_test(NAME test_kyfiledialog_signals COMMAND test_kyfiledialog_signals)
