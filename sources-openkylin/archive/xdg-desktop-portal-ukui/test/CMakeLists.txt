set(CMAKE_AUTOMOC on)

# 查找Qt组件
find_package(QT NAMES Qt6 Qt5 COMPONENTS Gui Test DBus Qml Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Test DBus Qml Widgets REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets Core Gui DBus LinguistTools REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KF5Wayland REQUIRED)

# 设置库文件名变量
include_directories(../src/filechooser)
include_directories(../src/utils)

set(XDP_UKUI_PC_PKGS gio-2.0 peony kysdk-waylandhelper ukui-search)
foreach(external_lib IN ITEMS ${XDP_UKUI_PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND XDP_UKUI_EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()


find_package(Qt5 COMPONENTS Widgets Core Gui DBus LinguistTools PrintSupport REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KF5Wayland REQUIRED)
find_package(KF5CoreAddons REQUIRED)
find_package(PkgConfig)


target_include_directories(xdg-desktop-portal-ukui PRIVATE
        ../src/filechooser
        ../src/../utils
        ${GIO_INCLUDE_DIRS}
        ${PEONY_INCLUDE_DIRS}
        ${KYSDK_WAYLANDHELPER_INCLUDE_DIRS}
        ${KYSDK_QTWIDGETS_INCLUDE_DIRS}

)


set(test-settings_SRCS
        ../src/filechooser/debug.h
        ../src/filechooser/kyfiledialog.cpp ../src/filechooser/kyfiledialog.h
        ../src/filechooser/kyfiledialogprivate.cpp ../src/filechooser/kyfiledialogprivate.h
        ../src/filechooser/menutoolbutoon.cpp ../src/filechooser/menutoolbutoon.h
        ../src/filechooser/pathbar.cpp ../src/filechooser/pathbar.h
        ../src/filechooser/sidebar.cpp ../src/filechooser/sidebar.h
        ../src/filechooser/uikyfiledialog.cpp ../src/filechooser/uikyfiledialog.h
        ../src/filechooser/file-operation-dialog/kyfiledialogrename.cpp ../src/filechooser/file-operation-dialog/kyfiledialogrename.h
        ../src/filechooser/file-operation-dialog/rename-editor.cpp ../src/filechooser/file-operation-dialog/rename-editor.h
        ../src/filechooser/file-operation-dialog/kyfileoperationdialog.cpp ../src/filechooser/file-operation-dialog/kyfileoperationdialog.h
        ../src/request.cpp ../src/request.h
        ../src/ukuidesktopportal.cpp ../src/ukuidesktopportal.h
        ../src/ukuifilechooserportal.cpp ../src/ukuifilechooserportal.h
        ../src/ukuisessioninhibitportal.cpp ../src/ukuisessioninhibitportal.h
        ../src/utils/windowutils.cpp ../src/utils/windowutils.h
#        ../src/xdg-desktop-portal-ukui.cpp
        ../src/utils/waylandintegration.h ../src/utils/waylandintegration.cpp
        ../src/print.h ../src/print.cpp

        test-settings.cpp)

    # 添加可执行文件test-settings
    add_executable(test-settings
        test-settings.cpp
        ${test-settings_SRCS})

# 链接Qt组件到可执行文件test-settings
target_link_libraries(test-settings PRIVATE Qt${QT_VERSION_MAJOR}::Test
    KF5::WindowSystem
    KF5::WaylandClient
    KF5::CoreAddons
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    Qt5::Widgets
    Qt5::PrintSupport
    ${XDP_UKUI_EXTERNAL_LIBS}
    ${GIO_LIBRARIES}
    ${PEONY_LIBRARIES}
    ${KYSDK_WAYLANDHELPER_LIBRARIES}
    ${KYSDK_QTWIDGETS_LIBRARIES}
    -lukui-log4qt
    Qt5::PrintSupportPrivate
    Qt5::GuiPrivate
    )

# 添加测试用例
add_test(NAME test-settings COMMAND test-settings)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
enable_testing()
