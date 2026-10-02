cmake_minimum_required(VERSION 3.14)
project(ukui-search-service VERSION 1.0.0 LANGUAGES C CXX)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(UKUI_SEARCH_SERVICE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Gui Quick Widgets RemoteObjects REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Gui Quick Widgets RemoteObjects REQUIRED)
find_package(PkgConfig REQUIRED)

set(GSETTINGS_PKG gsettings-qt)
if (QT_VERSION_MAJOR EQUAL 6)
    set(GSETTINGS_PKG gsettings-qt6)
endif ()

set(UKUI_SEARCH_SERVICE_EXTERNAL_LIBS "")
set(UKUI_SEARCH_SERVICE_PC_PKGS ${GSETTINGS_PKG})

foreach(PC_LIB IN ITEMS ${UKUI_SEARCH_SERVICE_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND UKUI_SEARCH_SERVICE_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

set(QRC_FILES qml/qml.qrc)
set(UKUI_SEARCH_SERVICE_SRC
        main.cpp
        ukui-search-service.cpp
        ukui-search-service.h
        monitor.cpp
        monitor.h
        cgroup-helper.cpp
        cgroup-helper.h
)

if (QT_VERSION_MAJOR EQUAL 5)
    qt5_generate_repc(UKUI_SEARCH_SERVICE_SRC ../libsearch/index/monitor.rep SOURCE)
endif ()

add_executable(ukui-search-service
        ${UKUI_SEARCH_SERVICE_SRC}
        ${QRC_FILES}
)

if (QT_VERSION_MAJOR EQUAL 6)
    qt6_add_repc_sources(${PROJECT_NAME} ../libsearch/index/monitor.rep)
endif ()

target_include_directories(ukui-search-service PRIVATE
    ../3rd-parties/qtsingleapplication/src
    ../libsearch
    ../libsearch/dirwatcher
    ../libsearch/filesystemwatcher
    ../libsearch/index
    ../libsearch/parser
)

target_compile_definitions(ukui-search-service PRIVATE
        QT_DEPRECATED_WARNINGS
        VERSION="${UKUI_SEARCH_SERVICE_VERSION}"
        QT_NO_KEYWORDS
        )

target_link_libraries(ukui-search-service PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::RemoteObjects
        ukui-search
        qtsingleapplication
        ${UKUI_SEARCH_SERVICE_EXTERNAL_LIBS}
)
install(FILES ../data/org.ukui.search.data.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
install(FILES ../data/ukui-search-service.desktop DESTINATION /etc/xdg/autostart)
install(TARGETS ukui-search-service DESTINATION /usr/bin)

if (BUILD_TEST)
    add_subdirectory(autotest)
endif ()
