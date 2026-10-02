cmake_minimum_required(VERSION 3.14)
project(ukui-search-app-data-service VERSION 1.0.0 LANGUAGES CXX)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(UKUI_SEARCH_APP_DATA_SERVICE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Sql Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Sql Network REQUIRED)
find_package(qt${QT_VERSION_MAJOR}xdg)
find_package(ukui-quick COMPONENTS platform REQUIRED)

find_package(PkgConfig REQUIRED)
set(EXTERNAL_LIBS "")
set(PC_PKGS sqlite3)

foreach(PC_LIB IN ITEMS ${PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()


set(UKUI_SEARCH_APP_DATA_SERVICE_SRC
        app-db-manager.cpp app-db-manager.h
        app-db-utils.cpp app-db-utils.h
        main.cpp
        pending-app-info.h
        pending-app-info-queue.cpp pending-app-info-queue.h
        signal-transformer.cpp signal-transformer.h
        ukui-search-app-data-service.cpp ukui-search-app-data-service.h
)

if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(UKUI_SEARCH_APP_DATA_SERVICE_SRC dbus/org.ukui.AppDatabase.xml app-db-manager.h UkuiSearch::AppDBManager)
else()
    qt5_add_dbus_adaptor(UKUI_SEARCH_APP_DATA_SERVICE_SRC dbus/org.ukui.AppDatabase.xml app-db-manager.h UkuiSearch::AppDBManager)
endif()

add_executable(ukui-search-app-data-service
        ${UKUI_SEARCH_APP_DATA_SERVICE_SRC}
)
target_include_directories(ukui-search-app-data-service PRIVATE
    ../3rd-parties/qtsingleapplication/src
    ../libchinese-segmentation
    ../libsearch
    ../libsearch/appdata
    ../libsearch/filesystemwatcher
)

target_compile_definitions(ukui-search-app-data-service PRIVATE
    QT_DEPRECATED_WARNINGS
    VERSION="${UKUI_SEARCH_APP_DATA_SERVICE_VERSION}"
)

target_link_libraries(ukui-search-app-data-service PRIVATE
        Qt${QT_VERSION_MAJOR}Xdg
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Sql
        chinese-segmentation
        ukui-search
        qtsingleapplication
        ukui-quick::platform
        ${EXTERNAL_LIBS}
        )

install(TARGETS ukui-search-app-data-service DESTINATION /usr/bin)
install(FILES ../data/ukui-search-app-data-service.desktop DESTINATION /etc/xdg/autostart)
install(FILES conf/com.ukui.search.appdb.service DESTINATION /usr/share/dbus-1/services/)
install(FILES conf/application-dirs.conf DESTINATION /etc/ukui/ukui-search/)

if (BUILD_TEST)
    add_subdirectory(autotest)
endif ()
