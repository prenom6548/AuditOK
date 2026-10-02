cmake_minimum_required(VERSION 3.5)

project(ukui-clipboard LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KF5_DEP_VERSION "5.54.0")

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

find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickWidgets REQUIRED)
find_package(KF5Config REQUIRED)
find_package(KF5GlobalAccel REQUIRED)
find_package(KF5I18n REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KF5CoreAddons REQUIRED)
find_package(Qt5 COMPONENTS Core Quick X11Extras Concurrent REQUIRED)
find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(ukui-quick COMPONENTS platform REQUIRED)

#translation
find_package(QT NAMES Qt6 Qt5 COMPONENTS LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools REQUIRED)

pkg_check_modules(GSETTINGQT REQUIRED gsettings-qt)

set(TS_FILES)
set(QM_FILES)

file(GLOB QML_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*")

list(APPEND TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/ukui-clipboard_zh_CN.ts)
list(APPEND TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/ukui-clipboard_bo_CN.ts)
list(APPEND TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/ukui-clipboard_zh_Hant.ts)
list(APPEND TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/ukui-clipboard_ug.ts)
list(APPEND TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/ukui-clipboard_mn.ts)
list(APPEND TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/ukui-clipboard_ky.ts)
list(APPEND TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/ukui-clipboard_kk.ts)
list(APPEND TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/ukui-clipboard_fr.ts)
list(APPEND TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/ukui-clipboard_es.ts)
list(APPEND TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/ukui-clipboard_de.ts)

set_source_files_properties(${TS_FILES}
    PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})

qt5_create_translation(QM_FILES ${TS_FILES} ${QML_FILES})
add_custom_target(lupdate_task DEPENDS ${TS_FILES})
add_custom_target(lrelease_task DEPENDS ${QM_FILES})

set(EXTERNAL_LIBS "")
set(PC_PKGS kysdk-waylandhelper kysdk-qtwidgets)
foreach(external_lib IN ITEMS ${PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()
add_subdirectory(third-party/qtsingleapplication)
include_directories(third-party/qtsingleapplication/src)

if(ANDROID)
    add_library(ukui-clipboard SHARED
      main.cpp
      qml.qrc
      ${QM_FILES}
    )
else()
    add_executable(ukui-clipboard
      main.cpp
      data.cpp
      data.h
      clipboardbackend.cpp
      clipboardbackend.h
      utils.cpp
      utils.h
      windowsettings.cpp
      windowsettings.h
      qml.qrc
      ${QM_FILES}
    )
endif()

target_compile_definitions(ukui-clipboard
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(ukui-clipboard PRIVATE
    Qt5::Core
    Qt5::Quick
    Qt5::X11Extras
    Qt5::QuickWidgets
    Qt5::Concurrent
    KF5::ConfigCore
    KF5::WindowSystem
    KF5::CoreAddons
    KF5::GlobalAccel
    qtsingleapplication
    Xtst
    kylin-ai-base
    ${X11_LIBRARIES}
    ${EXTERNAL_LIBS}
    ukui-quick::platform
    ${GSETTINGQT_LIBRARIES}
    -lukui-log4qt
)

install(TARGETS ukui-clipboard ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ukui-clipboard.desktop DESTINATION /etc/xdg/autostart/)

add_subdirectory(taskTray)
