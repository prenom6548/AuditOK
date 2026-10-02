cmake_minimum_required(VERSION 3.5)

project(ukui-intelligent-data-manager LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DCOMPLIE_AS_PROJECT)

if (SKIP_SEARCH_MATCH_ALL)
else()
        add_definitions(-DUKUI_SEARCH_HAS_MATCH_ALL)
endif()

if (BUILD_WITH_UKUI_SHELL)
    add_definitions(-DUSE_UKUI_SHELL)
endif()

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 COMPONENTS Core Gui Widgets X11Extras Quick LinguistTools Sql DBus RemoteObjects Test REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(ukui-quick COMPONENTS platform REQUIRED)

find_package(PkgConfig)
pkg_check_modules(PEONY REQUIRED peony)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(KYSDKFILESYSTEM REQUIRED kysdk-filesystem)
pkg_check_modules(UKUI_FILE_METADATA REQUIRED ukui-file-metadata)
pkg_check_modules(KYSDKQTWIDGETS_PKG REQUIRED kysdk-qtwidgets)
pkg_check_modules(KYSDKWAYLANDHELPER REQUIRED kysdk-waylandhelper)
pkg_check_modules(KYSDKALM REQUIRED kysdk-alm)
pkg_check_modules(UKUI_SEARCH REQUIRED ukui-search)
pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(QGSETTINGS REQUIRED gsettings-qt)
set(PKG_CONFIG_LIBS
    ${PEONY_LIBRARIES}
    ${GIO_LIBRARIES}
    ${KYSDKFILESYSTEM_LIBRARIES}
    ${UKUI_FILE_METADATA_LIBRARIES}
    ${KYSDKQTWIDGETS_PKG_LIBRARIES}
    ${KYSDKWAYLANDHELPER_LIBRARIES}
    ${KYSDKALM_LIBRARIES}
    ${UKUI_SEARCH_LIBRARIES}
    ${XCB_LIBRARIES}
    ${QGSETTINGS_LIBRARIES}
)

include_directories(
    ${PEONY_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${KYSDKFILESYSTEM_INCLUDE_DIRS}
    ${UKUI_FILE_METADATA_INCLUDE_DIRS}
    ${KYSDKQTWIDGETS_PKG_INCLUDE_DIRS}
    ${KYSDKWAYLANDHELPER_INCLUDE_DIRS}
    ${KYSDKALM_INCLUDE_DIRS}
    ${UKUI_SEARCH_INCLUDE_DIRS}
    ${XCB_INCLUDE_DIRS}
    ${QGSETTINGS_INCLUDE_DIRS}
    file
    file/qtremoteobject
    database
    condition
    model
    ui
    window
    thumbnail
)

# remote object
set(LIB_UKUI_IDM_FILE_SRC
  file/fileinfomanager.cpp
  file/fileinfo.cpp
  file/fileinfojob.cpp
  file/qtremoteobject/fileeventhandlerimpl.cpp
  condition/condition.cpp
)

qt5_generate_repc(LIB_UKUI_IDM_FILE_SRC file/qtremoteobject/fileeventhandler.rep SOURCE)

if (SHOULD_SKIP_TEST)
    message("skip tests")
else()
# testlib
enable_testing()

add_subdirectory(tests)

add_library(ukui-idm-fileinfo SHARED
  ${LIB_UKUI_IDM_FILE_SRC}
)

target_link_libraries(ukui-idm-fileinfo PRIVATE Qt5::Core Qt5::DBus Qt5::RemoteObjects ${PKG_CONFIG_LIBS})
endif()

# main program

set(TS_DIR ${CMAKE_SOURCE_DIR}/translations)
set(TS_FILES ${TS_DIR}/ukui-intelligent-data-manager_zh_CN.ts)

add_executable(ukui-intelligent-data-manager
  ui/main.cpp
  ui/main.qml
  ui/qml.qrc
  window/windowhelper.cpp
  file/fileop/fileoperationmanager.cpp
  model/sqlitemodel.cpp
  model/fileitemmodel.cpp
  model/fileitemproxysortfiltermodel.cpp
  model/spacesfilesfetcher.cpp
  model/modelcontroller.cpp
  model/previewhelper.cpp
  model/searchresultmodel.cpp
  model/spacessettings.cpp
  database/databasemanager.cpp
  condition/condition.cpp
  condition/conditionedit.cpp
  condition/conditioneditor.cpp
  condition/search/searchtask.cpp
  ${LIB_UKUI_IDM_FILE_SRC}
  thumbnail/thumbnailmanager.cpp
  ${TS_FILES}
)

target_compile_definitions(ukui-intelligent-data-manager
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(ukui-intelligent-data-manager
  PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::X11Extras Qt5::Quick Qt5::Sql Qt5::DBus Qt5::RemoteObjects KF5::WindowSystem ukui-quick::platform ${PKG_CONFIG_LIBS})

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
add_custom_target(qm_target ALL DEPENDS ${QM_FILES})
add_dependencies(ukui-intelligent-data-manager qm_target)

install(TARGETS ukui-intelligent-data-manager DESTINATION /usr/bin)
install(FILES ${QM_FILES} ${TS_FILES} DESTINATION /usr/share/ukui/idm)
install(FILES data/ukui-intelligent-data-manager.desktop DESTINATION /usr/share/applications)
install(FILES data/ukui-idm-backend.desktop DESTINATION /etc/xdg/autostart)
install(DIRECTORY data/hicolor DESTINATION /usr/share/icons)
