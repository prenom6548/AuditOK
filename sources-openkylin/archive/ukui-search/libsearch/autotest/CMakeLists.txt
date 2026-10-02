set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Test Gui Widgets DBus RemoteObjects Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Gui Widgets DBus RemoteObjects Concurrent)
find_package(qt5xdg)
find_package(PkgConfig REQUIRED)
pkg_check_modules(kysdk-qtwidgets REQUIRED IMPORTED_TARGET kysdk-qtwidgets)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GIO_UNIX REQUIRED IMPORTED_TARGET gio-unix-2.0)
pkg_check_modules(kysdk-systime REQUIRED IMPORTED_TARGET kysdk-systime)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(..)

add_executable(iconLoaderTest
        icon-loader-test.cpp
        ../icon-loader.cpp)

target_link_libraries(iconLoaderTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Gui
        ukui-search
        Qt5Xdg
        )
add_test(iconLoaderTest ${CMAKE_BINARY_DIR}/libsearch/autotest/iconLoaderTest)

add_executable(dataQueueTest
        data-queue-test.cpp)

target_link_libraries(dataQueueTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(dataQueueTest ${CMAKE_BINARY_DIR}/libsearch/autotest/dataQueueTest)

add_executable(searchPluginIfaceWhiteboxTest
        search-plugin-iface-whitebox-test.cpp
        ../plugininterface/search-plugin-iface.cpp
        ../plugininterface/action-transmiter.cpp)
target_link_libraries(searchPluginIfaceWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        )
add_test(searchPluginIfaceWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/searchPluginIfaceWhiteboxTest)

add_executable(resultItemTest
        result-item-test.cpp
        ../searchinterface/result-item.cpp)
target_link_libraries(resultItemTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(resultItemTest ${CMAKE_BINARY_DIR}/libsearch/autotest/resultItemTest)

add_executable(documentTest
        document-test.cpp
        ../index/document.cpp)
target_compile_definitions(documentTest PRIVATE QT_NO_KEYWORDS)
target_link_libraries(documentTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        xapian
        )
add_test(documentTest ${CMAKE_BINARY_DIR}/libsearch/autotest/documentTest)

add_executable(searchResultPropertyInfoTest
        search-result-property-info-test.cpp
        ../searchinterface/search-result-property-info.cpp)
target_link_libraries(searchResultPropertyInfoTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(searchResultPropertyInfoTest ${CMAKE_BINARY_DIR}/libsearch/autotest/searchResultPropertyInfoTest)

add_executable(actionTransmiterTest
        action-transmiter-test.cpp
        ../plugininterface/action-transmiter.cpp)
target_link_libraries(actionTransmiterTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        PkgConfig::kysdk-qtwidgets
        ukui-search
        )
add_test(actionTransmiterTest ${CMAKE_BINARY_DIR}/libsearch/autotest/actionTransmiterTest)

add_executable(actionLabelTest
        action-label-test.cpp
        ../plugininterface/action-label.cpp)
target_link_libraries(actionLabelTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        PkgConfig::kysdk-qtwidgets
        ukui-search
        )
add_test(actionLabelTest ${CMAKE_BINARY_DIR}/libsearch/autotest/actionLabelTest)

add_executable(searchControllerTest
        search-controller-test.cpp
        ../searchinterface/search-controller.cpp
        ../searchinterface/result-item.cpp)
target_link_libraries(searchControllerTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(searchControllerTest ${CMAKE_BINARY_DIR}/libsearch/autotest/searchControllerTest)

add_executable(appDataUtilsTest
        app-data-utils-test.cpp
        ../app-data-utils.cpp)
target_link_libraries(appDataUtilsTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(appDataUtilsTest ${CMAKE_BINARY_DIR}/libsearch/autotest/appDataUtilsTest)

add_executable(applicationPropertyHelperTest
        application-property-helper-test.cpp
        ../appdata/application-property-helper.cpp)
target_link_libraries(applicationPropertyHelperTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(applicationPropertyHelperTest ${CMAKE_BINARY_DIR}/libsearch/autotest/applicationPropertyHelperTest)

add_executable(applicationInfoLogUtilsTest
        application-info-log-utils-test.cpp
        ../log-utils.cpp)
target_compile_definitions(applicationInfoLogUtilsTest PRIVATE
        QT_NO_KEYWORDS)
target_link_libraries(applicationInfoLogUtilsTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        )
add_test(applicationInfoLogUtilsTest ${CMAKE_BINARY_DIR}/libsearch/autotest/applicationInfoLogUtilsTest)

add_executable(appInfoDbusArgumentTest
        app-info-dbus-argument-test.cpp)
target_link_libraries(appInfoDbusArgumentTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(appInfoDbusArgumentTest ${CMAKE_BINARY_DIR}/libsearch/autotest/appInfoDbusArgumentTest)

add_executable(pendingFileTest
        pending-file-test.cpp
        ../index/pending-file.cpp)
target_link_libraries(pendingFileTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(pendingFileTest ${CMAKE_BINARY_DIR}/libsearch/autotest/pendingFileTest)

add_executable(fileExtractionResultTest
        file-extraction-result-test.cpp
        ../index/file-extraction-result.cpp)
target_link_libraries(fileExtractionResultTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        ukui-file-metadata
        )
add_test(fileExtractionResultTest ${CMAKE_BINARY_DIR}/libsearch/autotest/fileExtractionResultTest)

add_executable(indexStatusRecorderTest
        index-status-recorder-test.cpp
        ../index/index-status-recorder.cpp)
target_link_libraries(indexStatusRecorderTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(indexStatusRecorderTest ${CMAKE_BINARY_DIR}/libsearch/autotest/indexStatusRecorderTest)

add_executable(indexSmallComponentsTest
        index-small-components-test.cpp)
target_compile_definitions(indexSmallComponentsTest PRIVATE QT_NO_KEYWORDS)
target_link_libraries(indexSmallComponentsTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        xapian
        )
add_test(indexSmallComponentsTest ${CMAKE_BINARY_DIR}/libsearch/autotest/indexSmallComponentsTest)

add_executable(indexDatabasePluginWhiteboxTest
        index-database-plugin-whitebox-test.cpp)
target_compile_definitions(indexDatabasePluginWhiteboxTest PRIVATE QT_NO_KEYWORDS)
target_link_libraries(indexDatabasePluginWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        PkgConfig::GIO
        ukui-search
        xapian
        )
add_test(indexDatabasePluginWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/indexDatabasePluginWhiteboxTest)
set_tests_properties(indexDatabasePluginWhiteboxTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.*=false;QT_FATAL_WARNINGS=0")

add_executable(fileContentIndexerWhiteboxTest
        file-content-indexer-whitebox-test.cpp
        ../index/file-content-indexer.cpp
        ../index/document.cpp)
target_compile_definitions(fileContentIndexerWhiteboxTest PRIVATE QT_NO_KEYWORDS)
target_link_libraries(fileContentIndexerWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ukui-file-metadata
        xapian
        )
add_test(fileContentIndexerWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/fileContentIndexerWhiteboxTest)

add_executable(fileSystemWatcherWhiteboxTest
        file-system-watcher-whitebox-test.cpp
        ../filesystemwatcher/file-system-watcher.cpp)
target_compile_definitions(fileSystemWatcherWhiteboxTest PRIVATE QT_NO_KEYWORDS)
target_link_libraries(fileSystemWatcherWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        ukui-search
        )
add_test(fileSystemWatcherWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/fileSystemWatcherWhiteboxTest)

add_executable(remoteFileEventHelperWhiteboxTest
        remote-file-event-helper-whitebox-test.cpp
        ../remote-file-event-helper.cpp)
target_compile_definitions(remoteFileEventHelperWhiteboxTest PRIVATE QT_NO_KEYWORDS)
target_link_libraries(remoteFileEventHelperWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::RemoteObjects
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
target_include_directories(remoteFileEventHelperWhiteboxTest PRIVATE
        ${CMAKE_BINARY_DIR}/libsearch)
add_test(remoteFileEventHelperWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/remoteFileEventHelperWhiteboxTest)

add_executable(fileWatcherWhiteboxTest
        file-watcher-whitebox-test.cpp
        ../index/file-watcher.cpp)
target_compile_definitions(fileWatcherWhiteboxTest PRIVATE QT_NO_KEYWORDS)
target_link_libraries(fileWatcherWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        PkgConfig::GIO
        PkgConfig::GLIB
        PkgConfig::GIO_UNIX
        ukui-search
        )
add_test(fileWatcherWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/fileWatcherWhiteboxTest)

add_executable(ukuiSearchQdbusWhiteboxTest
        ukui-search-qdbus-whitebox-test.cpp
        ../index/ukui-search-qdbus.cpp)
target_link_libraries(ukuiSearchQdbusWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Test
        )
add_test(ukuiSearchQdbusWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/ukuiSearchQdbusWhiteboxTest)

add_executable(dirWatcherTest
        dir-watcher-test.cpp
        ../dirwatcher/dir-watcher.cpp
        ../dirwatcher/search-dir-config.cpp)
target_link_libraries(dirWatcherTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(dirWatcherTest ${CMAKE_BINARY_DIR}/libsearch/autotest/dirWatcherTest)

add_executable(globalSettingsTest
        global-settings-test.cpp
        ../global-settings.cpp)
target_link_libraries(globalSettingsTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Test
        PkgConfig::GLIB
        PkgConfig::GIO
        PkgConfig::gsettings-qt
        ukui-search
        )
add_test(globalSettingsTest ${CMAKE_BINARY_DIR}/libsearch/autotest/globalSettingsTest)

add_executable(fileUtilsTest
        file-utils-test.cpp
        ../file-utils.cpp)
target_compile_definitions(fileUtilsTest PRIVATE QT_NO_KEYWORDS)
target_link_libraries(fileUtilsTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Test
        chinese-segmentation
        PkgConfig::GIO
        PkgConfig::GLIB
        PkgConfig::GIO_UNIX
        ukui-search
        )
add_test(fileUtilsTest ${CMAKE_BINARY_DIR}/libsearch/autotest/fileUtilsTest)

add_executable(separationLineTest
        separation-line-test.cpp
        ../plugininterface/separation-line.cpp)
target_link_libraries(separationLineTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ukui-search
        )
add_test(separationLineTest ${CMAKE_BINARY_DIR}/libsearch/autotest/separationLineTest)

add_executable(libsearchWhiteboxTest
        libsearch-whitebox-test.cpp)
target_link_libraries(libsearchWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Test
        PkgConfig::kysdk-systime
        ukui-search
        xapian
        )
add_test(libsearchWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/libsearchWhiteboxTest)
set_tests_properties(libsearchWhiteboxTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.*=false;QT_FATAL_WARNINGS=0;HOME=${CMAKE_BINARY_DIR}/libsearch/autotest/libsearch-whitebox-home;XDG_CONFIG_HOME=${CMAKE_BINARY_DIR}/libsearch/autotest/libsearch-whitebox-home/.config")

add_executable(searchPluginManagerWhiteboxTest
        search-plugin-manager-whitebox-test.cpp
        ../pluginmanage/search-plugin-manager.cpp)
target_include_directories(searchPluginManagerWhiteboxTest BEFORE PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/search-plugin-manager-stubs)
target_compile_definitions(searchPluginManagerWhiteboxTest PRIVATE
        SEARCH_PLUGIN_MANAGER_FIXTURE_DIR="${CMAKE_BINARY_DIR}/libsearch/autotest/plugin-manager-fixtures")
target_link_libraries(searchPluginManagerWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        )
add_dependencies(searchPluginManagerWhiteboxTest
        pluginManagerFixtureSearchPlugin
        pluginManagerFixtureWrongVersionSearchPlugin
        pluginManagerFixtureUnsupportedPlugin)
add_test(searchPluginManagerWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/searchPluginManagerWhiteboxTest)
set_tests_properties(searchPluginManagerWhiteboxTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.*=false;QT_FATAL_WARNINGS=0")

add_executable(appSearchTaskWhiteboxTest
        app-search-task-whitebox-test.cpp
        ../searchinterface/searchtasks/app-search-task.cpp
        ../searchinterface/search-controller.cpp
        ../searchinterface/result-item.cpp
        ../plugininterface/search-task-plugin-iface.h)
target_include_directories(appSearchTaskWhiteboxTest BEFORE PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app-search-task-stubs)
target_link_libraries(appSearchTaskWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        )
add_test(appSearchTaskWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/appSearchTaskWhiteboxTest)
set_tests_properties(appSearchTaskWhiteboxTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.*=false;QT_FATAL_WARNINGS=0")

add_executable(appSearchPluginWhiteboxTest
        app-search-plugin-whitebox-test.cpp
        app-search-plugin-stubs/global-settings.h
        ../appsearch/app-search-plugin.cpp
        ../searchinterface/result-item.cpp
        ../plugininterface/action-label.cpp
        ../plugininterface/separation-line.cpp
        ../plugininterface/search-plugin-iface.cpp
        ../plugininterface/action-transmiter.cpp)
target_compile_definitions(appSearchPluginWhiteboxTest PRIVATE QT_NO_KEYWORDS)
target_include_directories(appSearchPluginWhiteboxTest BEFORE PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app-search-plugin-stubs)
target_link_libraries(appSearchPluginWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        PkgConfig::GIO
        PkgConfig::kysdk-qtwidgets
        )
add_test(appSearchPluginWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/appSearchPluginWhiteboxTest)
set_tests_properties(appSearchPluginWhiteboxTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.*=false;QT_FATAL_WARNINGS=0")

add_executable(dataCollecterWhiteboxTest
        data-collecter-whitebox-test.cpp
        ../data-collecter.cpp)
target_include_directories(dataCollecterWhiteboxTest BEFORE PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/data-collecter-stubs)
target_link_libraries(dataCollecterWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        )
add_test(dataCollecterWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/dataCollecterWhiteboxTest)

add_executable(appInfoTableWhiteboxTest
        app-info-table-whitebox-test.cpp
        app-info-table-stubs/dbus-connector.h
        ../appdata/app-info-table.cpp
        ../appdata/app-info-table-private.h
        ../appdata/application-info.cpp
        ../appdata/application-property-helper.cpp
        ../app-data-utils.cpp)
target_include_directories(appInfoTableWhiteboxTest BEFORE PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/app-info-table-stubs
        ${CMAKE_CURRENT_SOURCE_DIR}/../appdata)
target_link_libraries(appInfoTableWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Test
        )
add_test(appInfoTableWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/appInfoTableWhiteboxTest)

add_executable(aiIndexerWhiteboxTest
        ai-indexer-whitebox-test.cpp
        ../index/ai-indexer.cpp)
target_include_directories(aiIndexerWhiteboxTest BEFORE PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ai-indexer-stubs)
target_link_libraries(aiIndexerWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        )
add_test(aiIndexerWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/aiIndexerWhiteboxTest)

add_executable(aiSearchPluginWhiteboxTest
        ai-search-plugin-whitebox-test.cpp
        ai-search-plugin-stubs/date-format-helper.h
        ai-search-plugin-stubs/global-settings.h
        ai-search-plugin-stubs/icon-loader.h
        ai-search-plugin-stubs/thumbnail-creator.h
        ../aisearch/ai-search-plugin.cpp
        ../plugininterface/action-label.cpp
        ../plugininterface/separation-line.cpp
        ../plugininterface/search-plugin-iface.cpp
        ../plugininterface/action-transmiter.cpp)
target_compile_definitions(aiSearchPluginWhiteboxTest PRIVATE QT_NO_KEYWORDS)
target_include_directories(aiSearchPluginWhiteboxTest BEFORE PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ai-search-plugin-stubs)
target_link_libraries(aiSearchPluginWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        PkgConfig::kysdk-qtwidgets
        PkgConfig::kysdk-systime
        )
add_test(aiSearchPluginWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/aiSearchPluginWhiteboxTest)
set_tests_properties(aiSearchPluginWhiteboxTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.*=false;QT_FATAL_WARNINGS=0")

add_executable(indexJobsWhiteboxTest
        index-jobs-whitebox-test.cpp
        ../index/batch-indexer.cpp
        ../index/index-scheduler.cpp
        ../index/index-updater.cpp)
target_compile_definitions(indexJobsWhiteboxTest PRIVATE QT_NO_KEYWORDS)
target_link_libraries(indexJobsWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Concurrent
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        PkgConfig::GIO
        PkgConfig::GLIB
        PkgConfig::GIO_UNIX
        PkgConfig::gsettings-qt
        ukui-search
        xapian
        )
add_test(indexJobsWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/indexJobsWhiteboxTest)
set_tests_properties(indexJobsWhiteboxTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.*=false;QT_FATAL_WARNINGS=0")

add_executable(pluginManagerWhiteboxTest
        plugin-manager-whitebox-test.cpp
        ../pluginmanage/plugin-manager.cpp
        ../plugininterface/search-task-plugin-iface.h)
target_compile_definitions(pluginManagerWhiteboxTest PRIVATE
        PLUGIN_INSTALL_DIRS="${CMAKE_BINARY_DIR}/libsearch/autotest/plugin-manager-plugins"
        PLUGIN_MANAGER_FIXTURE_DIR="${CMAKE_BINARY_DIR}/libsearch/autotest/plugin-manager-fixtures")
target_link_libraries(pluginManagerWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        )
add_dependencies(pluginManagerWhiteboxTest
        pluginManagerFixtureSearchPlugin
        pluginManagerFixtureWrongVersionSearchPlugin
        pluginManagerFixtureTaskPlugin
        pluginManagerFixtureUnsupportedPlugin)
add_test(pluginManagerWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/pluginManagerWhiteboxTest)

add_library(pluginManagerFixtureSearchPlugin MODULE
        plugin-manager-fixtures/valid-search-plugin.cpp)
target_compile_definitions(pluginManagerFixtureSearchPlugin PRIVATE QT_NO_KEYWORDS)
target_link_libraries(pluginManagerFixtureSearchPlugin PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        )
set_target_properties(pluginManagerFixtureSearchPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libsearch/autotest/plugin-manager-fixtures
        OUTPUT_NAME fixture-search-plugin
        PREFIX "")

add_library(pluginManagerFixtureWrongVersionSearchPlugin MODULE
        plugin-manager-fixtures/wrong-version-search-plugin.cpp)
target_compile_definitions(pluginManagerFixtureWrongVersionSearchPlugin PRIVATE QT_NO_KEYWORDS)
target_link_libraries(pluginManagerFixtureWrongVersionSearchPlugin PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        )
set_target_properties(pluginManagerFixtureWrongVersionSearchPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libsearch/autotest/plugin-manager-fixtures
        OUTPUT_NAME fixture-wrong-version-search-plugin
        PREFIX "")

add_library(pluginManagerFixtureTaskPlugin MODULE
        plugin-manager-fixtures/valid-task-plugin.cpp)
target_compile_definitions(pluginManagerFixtureTaskPlugin PRIVATE QT_NO_KEYWORDS)
target_link_libraries(pluginManagerFixtureTaskPlugin PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        ukui-search
        )
set_target_properties(pluginManagerFixtureTaskPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libsearch/autotest/plugin-manager-fixtures
        OUTPUT_NAME fixture-task-plugin
        PREFIX "")

add_library(pluginManagerFixtureUnsupportedPlugin MODULE
        plugin-manager-fixtures/unsupported-plugin.cpp)
target_link_libraries(pluginManagerFixtureUnsupportedPlugin PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        )
set_target_properties(pluginManagerFixtureUnsupportedPlugin PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libsearch/autotest/plugin-manager-fixtures
        OUTPUT_NAME fixture-unsupported-plugin
        PREFIX "")

add_executable(pluginKeywordSearchWhiteboxTest
        plugin-keyword-search-whitebox-test.cpp)
target_compile_definitions(pluginKeywordSearchWhiteboxTest PRIVATE QT_NO_KEYWORDS)
target_link_libraries(pluginKeywordSearchWhiteboxTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Xml
        PkgConfig::gsettings-qt
        PkgConfig::kysdk-systime
        ukui-search
        xapian
        )
add_test(pluginKeywordSearchWhiteboxTest ${CMAKE_BINARY_DIR}/libsearch/autotest/pluginKeywordSearchWhiteboxTest)
set_tests_properties(pluginKeywordSearchWhiteboxTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.*=false;QT_FATAL_WARNINGS=0")
