cmake_minimum_required(VERSION 3.16)

#project(ukui-task-manager LANGUAGES CXX)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(UKUI_TASK_MANAGER_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core LinguistTools DBus Widgets Quick Qml Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core LinguistTools DBus Widgets Quick Qml Concurrent REQUIRED)
if (QT_VERSION_MAJOR EQUAL "5")
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS X11Extras REQUIRED)
    find_package(KF5WindowSystem REQUIRED)
    find_package(KF5Wayland)
    find_package(Qt5Xdg REQUIRED)
elseif (QT_VERSION_MAJOR EQUAL "6")
    find_package(KF6WindowSystem REQUIRED)
    find_package(KWayland)
    find_package(Qt6Xdg REQUIRED)
endif()
find_package(ukui-quick COMPONENTS core framework platform)
find_package(ukui-file-metadata)

find_package(PkgConfig REQUIRED)
set(UKUI_TASK_MANAGER_PC_PKGS kysdk-waylandhelper ukui-search gio-2.0)
foreach(external_lib IN ITEMS ${UKUI_TASK_MANAGER_PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND UKUI_TASK_MANAGER_EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()

file(GLOB TS_FILES translations/*.ts)

set(TRANSLATION_FILE_DIR "/usr/share/ukui-task-manager/translations")
add_compile_definitions(TRANSLATION_FILE_DIR="${TRANSLATION_FILE_DIR}"
        UKUI_TASK_MANAGER_VERSION="${UKUI_TASK_MANAGER_VERSION}")

set(ukuiTaskManager_SRC
        ukui-task-manager.cpp ukui-task-manager.h
        task-manager-item.cpp task-manager-item.h
        utils.cpp utils.h
        actions.cpp actions.h
        recent-files-service.cpp recent-files-service.h
        task-manager-plugin.cpp task-manager-plugin.h
        task-manager-filter-model.cpp
        task-manager-filter-model.h
        widget-delegate.h widget-delegate.cpp
)
if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(ukuiTaskManager_SRC dbus/org.ukui.TaskManager.xml ukui-task-manager.h TaskManager::UkuiTaskManager)
else()
    qt5_add_dbus_adaptor(ukuiTaskManager_SRC dbus/org.ukui.TaskManager.xml ukui-task-manager.h TaskManager::UkuiTaskManager)
endif()

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
endif()

set(QRC_FILES container.qrc qml/qml.qrc)

add_library(ukui-task-manager SHARED
        ${ukuiTaskManager_SRC}
        ${QRC_FILES}
        ${QM_FILES}
        )
target_include_directories(ukui-task-manager PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/framework/config>
)

target_link_libraries(ukui-task-manager PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Concurrent
        ${UKUI_TASK_MANAGER_EXTERNAL_LIBS}
        KF${QT_VERSION_MAJOR}::WindowSystem
        ukui-quick::core
        ukui-quick::framework
        ukui-quick::platform
        ukui-file-metadata
 )
if (QT_VERSION_MAJOR EQUAL "5")
    target_link_libraries(ukui-task-manager PRIVATE Qt::X11Extras Qt5Xdg KF5::WaylandClient)
else()
    target_link_libraries(ukui-task-manager PRIVATE Qt6Xdg Plasma::KWaylandClient)
endif()

set_target_properties(ukui-task-manager PROPERTIES
        VERSION ${UKUI_TASK_MANAGER_VERSION}
        SOVERSION ${VERSION_MAJOR}
        OUTPUT_NAME ukui-task-manager
        )

install(FILES ${QM_FILES} DESTINATION "${TRANSLATION_FILE_DIR}")

set(PLUGIN_INSTALL_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/panel/taskManager")

install(DIRECTORY "plugin/" DESTINATION "/usr/share/ukui/widgets/org.ukui.panel.taskManager")
install(FILES "qmldir" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/AppIcon.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/AppList.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/TaskManagerBase.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/TaskItemData.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/TaskManagerView.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/ThumbnailWindow.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/ThumbnailContainer.qml" DESTINATION ${PLUGIN_INSTALL_PATH})

install(TARGETS "ukui-task-manager" LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES ukui-panel-taskManager-v11.json DESTINATION "/opt/system/conf/ukui-security-support/")

if(BUILD_TEST)
    add_subdirectory(autotest)
endif()
