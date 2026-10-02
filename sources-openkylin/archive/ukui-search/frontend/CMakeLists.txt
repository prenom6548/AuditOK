cmake_minimum_required(VERSION 3.14)
project(frontend VERSION 1.0 LANGUAGES C CXX)

set(VERSION_MAJOR 2)
set(VERSION_MINOR 2)
set(VERSION_MICRO 3)
set(UKUI_SEARCH_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Widgets Xml Concurrent Sql LinguistTools Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Widgets Xml Concurrent Sql LinguistTools Quick REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(ukui-quick COMPONENTS platform REQUIRED)

set(X11EXTRA_PKG Qt${QT_VERSION_MAJOR}X11Extras)
if (QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS X11Extras REQUIRED)
elseif (QT_VERSION_MAJOR EQUAL 6)
    set(X11EXTRA_PKG "")
endif()

set(GSETTINGS_PKG gsettings-qt)
if (QT_VERSION_MAJOR EQUAL 6)
    set(GSETTINGS_PKG gsettings-qt6)
endif ()

set(UKUI_SEARCH_EXTERNAL_LIBS "")
set(UKUI_SEARCH_PC_PKGS ${GSETTINGS_PKG})

foreach(PC_LIB IN ITEMS ${UKUI_SEARCH_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND UKUI_SEARCH_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

set(UKUI_SEARCH_SRC
        control/create-index-ask-dialog.cpp control/create-index-ask-dialog.h
        control/list-labels/show-more-label.cpp control/list-labels/show-more-label.h
        control/list-labels/title-label.cpp control/list-labels/title-label.h
        control/search-line-edit.cpp control/search-line-edit.h
        control/stack-pages/search-page-section.cpp control/stack-pages/search-page-section.h
        control/stack-pages/search-result-page.cpp control/stack-pages/search-result-page.h
        main.cpp
        mainwindow.cpp mainwindow.h
        model/best-list-model.cpp model/best-list-model.h
        model/search-result-manager.cpp model/search-result-manager.h
        model/search-result-model.cpp model/search-result-model.h
        model/one-result.cpp model/one-result.h
        search-app-widget-plugin/search.cpp search-app-widget-plugin/search.h
        ukui-search-dbus-service.cpp ukui-search-dbus-service.h
        ukui-search-gui.cpp ukui-search-gui.h
        view/best-list-view.cpp view/best-list-view.h
        view/result-view.cpp view/result-view.h
        view/result-view-delegate.cpp view/result-view-delegate.h
        )
if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(UKUI_SEARCH_SRC org.ukui.search.service.xml ukui-search-dbus-service.h UkuiSearch::UkuiSearchDbusServices)
    qt_add_dbus_interface(UKUI_SEARCH_SRC org.ukui.search.service.xml service_interface)
else()
    qt5_add_dbus_adaptor(UKUI_SEARCH_SRC org.ukui.search.service.xml ukui-search-dbus-service.h UkuiSearch::UkuiSearchDbusServices)
    qt5_add_dbus_interface(UKUI_SEARCH_SRC org.ukui.search.service.xml service_interface)
endif()

set(QRC_FILES resource.qrc)

file(GLOB UKUI_SEARCH_TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../translations/ukui-search/*.ts)
set_source_files_properties(${UKUI_SEARCH_TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_BINARY_DIR}/frontend/.qm)

file(GLOB UKUI_SEARCH_APP_WIDGET_TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../translations/ukui-search/appwidget/*.ts)
set_source_files_properties(${UKUI_SEARCH_APP_WIDGET_TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_BINARY_DIR}/frontend/.qm)

if(QT_VERSION_MAJOR EQUAL 5)
    qt5_create_translation(UKUI_SEARCH_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${UKUI_SEARCH_TS_FILES})
    qt5_create_translation(UKUI_SEARCH_APP_WIDGET_QM_FILES ${PROJECT_SOURCE_DIR} ${UKUI_SEARCH_APP_WIDGET_TS_FILES})
elseif (QT_VERSION_MAJOR EQUAL 6)
    qt6_create_translation(UKUI_SEARCH_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${UKUI_SEARCH_TS_FILES})
    qt6_create_translation(UKUI_SEARCH_APP_WIDGET_QM_FILES ${PROJECT_SOURCE_DIR} ${UKUI_SEARCH_APP_WIDGET_TS_FILES})
endif ()

add_executable(frontend
        ${UKUI_SEARCH_SRC}
        ${QRC_FILES}
        ${UKUI_SEARCH_QM_FILES}
        ${UKUI_SEARCH_APP_WIDGET_QM_FILES})
set_target_properties(frontend
        PROPERTIES
        OUTPUT_NAME ukui-search
)
set(UKUI_SEARCH_QM_INSTALL_PATH /usr/share/ukui-search/translations/ukui-search)
target_compile_definitions(frontend PRIVATE
        VERSION="${UKUI_SEARCH_VERSION}"
        UKUI_SEARCH_QM_INSTALL_PATH="${UKUI_SEARCH_QM_INSTALL_PATH}"
        )

target_link_libraries(frontend PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        ${X11EXTRA_PKG}
        Qt${QT_VERSION_MAJOR}::Quick
        ukui-appwidget-manager
        ukui-appwidget-provider
        ukui-search
        chinese-segmentation
        qtsingleapplication
        perftools
        ukui-quick::platform
        ${UKUI_SEARCH_EXTERNAL_LIBS}
        )

target_include_directories(frontend PRIVATE
        ../3rd-parties/qtsingleapplication/src
        ../perftools/src
        ../libchinese-segmentation
        ../libsearch
        ../libsearch/pluginmanage
        ../libsearch/plugininterface
        ../libsearch/searchinterface
        control
        control/list-labels
        control/stack-pages
        model
        search-app-widget-plugin
        view
        )

set(APP_WIDGET_FILES_PRE
    search-app-widget-plugin/provider
)
#小插件相关
install(FILES ${APP_WIDGET_FILES_PRE}/data/search.conf DESTINATION /usr/share/appwidget/config/)
install(FILES ${APP_WIDGET_FILES_PRE}/org.ukui.appwidget.provider.search.service DESTINATION /usr/share/dbus-1/services/)
install(FILES
        ${APP_WIDGET_FILES_PRE}/data/search.png
        ${APP_WIDGET_FILES_PRE}/data/ukui-search.svg
        DESTINATION /usr/share/appwidget/search/)
install(FILES ${APP_WIDGET_FILES_PRE}/data/search.qml DESTINATION /usr/share/appwidget/qml/)
install(FILES ${UKUI_SEARCH_APP_WIDGET_QM_FILES} DESTINATION /usr/share/appwidget/translations/)
#二进制
install(TARGETS frontend RUNTIME DESTINATION /usr/bin)
#翻译
install(FILES ${UKUI_SEARCH_QM_FILES} DESTINATION ${UKUI_SEARCH_QM_INSTALL_PATH})
#desktop文件
install(FILES ../data/ukui-search-menu.desktop DESTINATION /usr/share/applications)
install(FILES ../data/ukui-search.desktop DESTINATION /etc/xdg/autostart)
#gsettings
install(FILES ../data/org.ukui.log4qt.ukui-search.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
#域控能力集文件
install(FILES ../data/ukui-search.json DESTINATION /opt/system/conf/ukui-security-support/)

if(BUILD_TEST)
    enable_testing()
    add_subdirectory(autotest)
endif ()
