set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Test REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)
pkg_check_modules(GIO_UNIX REQUIRED IMPORTED_TARGET gio-unix-2.0)

include_directories(../dirwatcher)

add_executable(searchDirTest
        search-dir-test.cpp
        ../dirwatcher/volume-manager.h
        ../dirwatcher/search-dir.cpp
        ../dirwatcher/config.cpp
        volume-manager-test-stub.cpp)
target_link_libraries(searchDirTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        PkgConfig::GIO
        PkgConfig::GIO_UNIX)
add_test(searchDirTest ${CMAKE_BINARY_DIR}/ukui-search-service-dir-manager/autotest/searchDirTest)

add_executable(configTest
        config-test.cpp
        ../dirwatcher/config.cpp
        ../dirwatcher/search-dir.cpp
        ../dirwatcher/volume-manager.cpp)
target_link_libraries(configTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        PkgConfig::GIO
        PkgConfig::GIO_UNIX)
add_test(configTest ${CMAKE_BINARY_DIR}/ukui-search-service-dir-manager/autotest/configTest)

add_executable(volumeTest
        volume-test.cpp
        ../dirwatcher/volume-manager.cpp
        gio-stubs.c)
target_link_libraries(volumeTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        PkgConfig::GIO
        PkgConfig::GIO_UNIX)
add_test(volumeTest ${CMAKE_BINARY_DIR}/ukui-search-service-dir-manager/autotest/volumeTest)

set(DIR_WATCHER_SERVICE_TEST_SRC
        dir-watcher-service-test.cpp
        ../dirwatcher/dir-watcher.cpp
        ../dirwatcher/search-dir.cpp
        ../dirwatcher/config.cpp
        ../dirwatcher/volume-manager.cpp)

if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(DIR_WATCHER_SERVICE_TEST_SRC ../dirwatcher/com.ukui.search.FileIndexService.xml ../dirwatcher/dir-watcher.h DirWatcher)
else()
    qt5_add_dbus_adaptor(DIR_WATCHER_SERVICE_TEST_SRC ../dirwatcher/com.ukui.search.FileIndexService.xml ../dirwatcher/dir-watcher.h DirWatcher)
endif()

add_executable(dirWatcherServiceTest ${DIR_WATCHER_SERVICE_TEST_SRC})
target_include_directories(dirWatcherServiceTest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(dirWatcherServiceTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Test
        PkgConfig::GIO
        PkgConfig::GIO_UNIX)
add_test(dirWatcherServiceTest ${CMAKE_BINARY_DIR}/ukui-search-service-dir-manager/autotest/dirWatcherServiceTest)
