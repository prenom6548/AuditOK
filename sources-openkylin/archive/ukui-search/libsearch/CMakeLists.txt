cmake_minimum_required(VERSION 3.14)
project(ukui-search VERSION 1.0 LANGUAGES C CXX)

set(VERSION_MAJOR 2)
set(VERSION_MINOR 3)
set(VERSION_MICRO 0)
set(LIBUKUI_SEARCH_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Widgets Xml Concurrent Sql LinguistTools RemoteObjects REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Widgets Xml Concurrent Sql LinguistTools RemoteObjects REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(qt${QT_VERSION_MAJOR}xdg)
find_package(ukui-file-metadata)
find_package(xapian)
find_package(ukui-quick COMPONENTS platform REQUIRED)
find_package(KysdkAiCommon)

set(GSETTINGS_PKG gsettings-qt)
if (QT_VERSION_MAJOR EQUAL 6)
    set(GSETTINGS_PKG gsettings-qt6)
endif ()

set(LIBUKUI_SEARCH_EXTERNAL_LIBS "")
set(LIBUKUI_SEARCH_PC_PKGS
        gio-2.0
        glib-2.0
        gio-unix-2.0
        ${GSETTINGS_PKG}
        kysdk-qtwidgets
        kysdk-systime
        kysdk-datacollect
        kyai-data-management-client
        )

foreach(PC_LIB IN ITEMS ${LIBUKUI_SEARCH_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND LIBUKUI_SEARCH_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

set(LIBUKUI_SEARCH_SRC
        appdata/app-info-dbus-argument.h
        appdata/app-info-table.cpp appdata/app-info-table.h
        appdata/app-info-table-private.h
        appdata/sqlite-database-utils.cpp appdata/sqlite-database-utils.h
        appdata/application-info.cpp appdata/application-info.h
        appdata/application-property.h
        appdata/application-property-helper.cpp appdata/application-property-helper.h
        appsearch/app-search-plugin.cpp appsearch/app-search-plugin.h
        common.h
        dirwatcher/dir-watcher.cpp dirwatcher/dir-watcher.h
        dirwatcher/search-dir-config.cpp dirwatcher/search-dir-config.h
        file-utils.cpp file-utils.h
        filesystemwatcher/file-system-watcher.cpp filesystemwatcher/file-system-watcher.h
        filesystemwatcher/file-system-watcher-private.h
        global-settings.cpp global-settings.h
        global-settings-private.h
        gobject-template.cpp gobject-template.h
        index/basic-indexer.cpp index/basic-indexer.h
        index/batch-indexer.cpp index/batch-indexer.h
        index/ai-indexer.cpp index/ai-indexer.h
        index/compatible-define.h
        index/database.cpp index/database.h
        index/document.cpp index/document.h
        index/file-content-indexer.cpp index/file-content-indexer.h
        index/file-indexer-config.cpp index/file-indexer-config.h
        index/file-reader.cpp index/file-reader.h
        index/file-search-plugin.cpp index/file-search-plugin.h
        index/file-watcher.cpp index/file-watcher.h
        index/index-scheduler.cpp index/index-scheduler.h
        index/index-status-recorder.cpp index/index-status-recorder.h
        index/index-updater.cpp index/index-updater.h
        index/pending-file.cpp index/pending-file.h
        index/pending-file-queue.cpp index/pending-file-queue.h
        index/search-manager.cpp index/search-manager.h
        index/ukui-search-qdbus.cpp index/ukui-search-qdbus.h
        index/writable-database.cpp index/writable-database.h
        index/file-extraction-result.cpp
        index/file-extraction-result.h
        index/index-monitor.cpp
        libsearch_global.h
        log-utils.cpp log-utils.h
        notesearch/note-search-plugin.cpp notesearch/note-search-plugin.h
        plugininterface/action-label.cpp plugininterface/action-label.h
        plugininterface/action-transmiter.cpp plugininterface/action-transmiter.h
        plugininterface/data-queue.h
        plugininterface/plugin-iface.h
        plugininterface/search-plugin-iface.cpp plugininterface/search-plugin-iface.h
        plugininterface/search-task-plugin-iface.h
        plugininterface/separation-line.cpp plugininterface/separation-line.h
        pluginmanage/plugin-info.h
        pluginmanage/plugin-manager.cpp pluginmanage/plugin-manager.h
        pluginmanage/search-plugin-manager.cpp pluginmanage/search-plugin-manager.h
        pluginmanage/search-task-plugin-manager.cpp pluginmanage/search-task-plugin-manager.h
        searchinterface/result-item.cpp searchinterface/result-item.h
        searchinterface/search-controller.cpp searchinterface/search-controller.h
        searchinterface/search-result-property.h
        searchinterface/search-result-property-info.cpp searchinterface/search-result-property-info.h
        searchinterface/searchtasks/app-search-task.cpp searchinterface/searchtasks/app-search-task.h
        searchinterface/searchtasks/file-content-search-task.cpp searchinterface/searchtasks/file-content-search-task.h
        searchinterface/searchtasks/file-search-task.cpp searchinterface/searchtasks/file-search-task.h
        searchinterface/ukui-search-task.cpp searchinterface/ukui-search-task.h
        settingsearch/settings-search-plugin.cpp settingsearch/settings-search-plugin.h
        websearch/web-search-plugin.cpp websearch/web-search-plugin.h
        aisearch/ai-search-plugin.cpp aisearch/ai-search-plugin.h
        thumbnail-creator.cpp thumbnail-creator.h
        icon-loader.cpp icon-loader.h
        data-collecter.cpp
        data-collecter.h
        remote-file-event-helper.cpp
        date-format-helper.h
        app-data-utils.h
        app-data-utils.cpp
        security-control-config.cpp
        security-control-config.h
)

set(QRC_FILES resource1.qrc)
file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../translations/libukui-search/*.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_BINARY_DIR}/libsearch/.qm)

if (QT_VERSION_MAJOR EQUAL 5)
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
    qt5_generate_repc(LIBUKUI_SEARCH_SRC index/monitor.rep REPLICA)
    qt5_generate_repc(LIBUKUI_SEARCH_SRC fileeventhandler.rep REPLICA)
elseif (QT_VERSION_MAJOR EQUAL 6)
    qt6_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
endif()

add_library(${PROJECT_NAME} SHARED
        ${LIBUKUI_SEARCH_SRC}
        ${QRC_FILES}
        ${QM_FILES}
)

if (QT_VERSION_MAJOR EQUAL 6)
    qt6_add_repc_replicas(${PROJECT_NAME} index/monitor.rep)
    qt6_add_repc_replicas(${PROJECT_NAME} fileeventhandler.rep)
endif()

set(HEADERS
        libsearch_global.h
        plugininterface/action-label.h
        plugininterface/action-transmiter.h
        plugininterface/data-queue.h
        plugininterface/plugin-iface.h
        plugininterface/search-plugin-iface.h
        plugininterface/search-task-plugin-iface.h
        plugininterface/separation-line.h
        appdata/application-info.h
        appdata/application-property.h
        appdata/application-property-helper.h
        searchinterface/ukui-search-task.h
        searchinterface/result-item.h
        searchinterface/search-result-property.h
        searchinterface/search-controller.h
        filesystemwatcher/file-system-watcher.h
        development-files/header-files/UkuiSearchTask
        development-files/header-files/UkuiSearchPluginIface
        development-files/header-files/FileSystemWatcher
        index/index-monitor.h
        dirwatcher/search-dir-config.h
        )

include_directories(
    ../libchinese-segmentation
    appdata
    appsearch
    dirwatcher
    filesystemwatcher
    index
    notesearch
    parser
    plugininterface
    pluginmanage
    searchinterface
    searchinterface/searchtasks
    settingsearch
    websearch
    aisearch
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        PLUGIN_INSTALL_DIRS="/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/ukui-search-plugins"
        QT_NO_KEYWORDS
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}Xdg
        Qt${QT_VERSION_MAJOR}::Concurrent
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Xml
        Qt${QT_VERSION_MAJOR}::RemoteObjects
        chinese-segmentation
        xapian
        ukui-file-metadata
        ukui-quick::platform
        kysdk-ai-common
        ${LIBUKUI_SEARCH_EXTERNAL_LIBS}
        )

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/ukui-search")
set(HEADERS_INSTALL_DIR /usr/include/ukui-search)
set(PC_INSTALL_DIR "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig")

target_include_directories(${PROJECT_NAME}  PUBLIC  $<INSTALL_INTERFACE:${HEADERS_INSTALL_DIR}>)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-search-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-search-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR})
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-search-config-version.cmake
        VERSION ${LIBUKUI_SEARCH_VERSION}
        COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-search.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/ukui-search.pc"
        INSTALL_DESTINATION  ${PC_INSTALL_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${LIBUKUI_SEARCH_VERSION}
        SOVERSION ${VERSION_MAJOR}
        OUTPUT_NAME ukui-search
        )
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${HEADERS_INSTALL_DIR}
        LIBRARY DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        )
install(EXPORT ${PROJECT_NAME}
        FILE ${PROJECT_NAME}-targets.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
install(FILES ${QM_FILES} DESTINATION /usr/share/ukui-search/translations/libukui-search)

install(FILES ${HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/ukui-search.pc DESTINATION ${PC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-search-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-search-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
enable_testing()
if(BUILD_TEST)
    add_subdirectory(test)
    add_subdirectory(autotest)
endif ()
