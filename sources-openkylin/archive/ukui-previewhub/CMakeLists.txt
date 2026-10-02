# UKUI Preview Hub — standalone shared library (preview logic in core/, no Peony dependency).
cmake_minimum_required(VERSION 3.16)
project(ukui-previewhub VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

set(UKUI_PREVIEWHUB_WEBENGINE_ARCHES
    x86_64 amd64 aarch64 arm64
)

set(UKUI_PREVIEWHUB_TARGET_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
if(DEFINED ENV{DEB_HOST_ARCH} AND NOT "$ENV{DEB_HOST_ARCH}" STREQUAL "")
    set(UKUI_PREVIEWHUB_TARGET_ARCH "$ENV{DEB_HOST_ARCH}")
endif()

if(UKUI_PREVIEWHUB_TARGET_ARCH IN_LIST UKUI_PREVIEWHUB_WEBENGINE_ARCHES)
    set(UKUI_PREVIEWHUB_ENABLE_WEBENGINE_PREVIEW ON)
else()
    set(UKUI_PREVIEWHUB_ENABLE_WEBENGINE_PREVIEW OFF)
endif()

message(STATUS "ukui-previewhub target arch: ${UKUI_PREVIEWHUB_TARGET_ARCH}")
message(STATUS "ukui-previewhub WebEngine preview: ${UKUI_PREVIEWHUB_ENABLE_WEBENGINE_PREVIEW}")

find_package(Qt5 REQUIRED COMPONENTS
    Core Gui Widgets Multimedia MultimediaWidgets
)

if(UKUI_PREVIEWHUB_ENABLE_WEBENGINE_PREVIEW)
    find_package(Qt5 REQUIRED COMPONENTS
        WebEngine WebEngineCore WebEngineWidgets WebChannel
    )
endif()

find_package(KF5SyntaxHighlighting REQUIRED)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/ukui-previewhub.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ukui-previewhub.pc"
    @ONLY
)

set(CORE "${CMAKE_CURRENT_SOURCE_DIR}/core")

set(UKUI_PREVIEWHUB_SOURCES
    src/ukui_previewhub_api.cpp
    core/previewhub_fileinfo.cpp
    core/previewhub_filewatcher.cpp
    core/preview-file-type.cpp
    core/preview-page-manger.cpp
    core/content-preview-page.cpp
    core/previewpage/audio.cpp
    core/previewpage/base.cpp
    core/previewpage/image.cpp
    core/previewpage/other.cpp
    core/previewpage/text.cpp
    core/previewpage/video.cpp
    core/components/icon-button.cpp
    core/components/slider.cpp
    core/components/preview-control-bar.cpp
    core/components/media-control-bar.cpp
    core/resources/resources.qrc
)

set(UKUI_PREVIEWHUB_TRANSLATIONS
    translations/ukui-previewhub_zh_CN.ts
)

find_program(LRELEASE_EXECUTABLE NAMES lrelease REQUIRED)
foreach(translation_file IN LISTS UKUI_PREVIEWHUB_TRANSLATIONS)
    get_filename_component(translation_name "${translation_file}" NAME_WE)
    set(translation_qm
        "${CMAKE_CURRENT_BINARY_DIR}/translations/${translation_name}.qm")
    add_custom_command(
        OUTPUT "${translation_qm}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${CMAKE_CURRENT_BINARY_DIR}/translations"
        COMMAND "${LRELEASE_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${translation_file}"
            -qm "${translation_qm}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${translation_file}"
        VERBATIM
    )
    list(APPEND UKUI_PREVIEWHUB_TRANSLATION_QMS "${translation_qm}")
endforeach()

add_custom_target(ukui-previewhub-translations ALL
    DEPENDS ${UKUI_PREVIEWHUB_TRANSLATION_QMS}
)

if(UKUI_PREVIEWHUB_ENABLE_WEBENGINE_PREVIEW)
    list(APPEND UKUI_PREVIEWHUB_SOURCES
        core/libreoffice-runtime.cpp
        core/previewpage/html.cpp
        core/previewpage/pdf2.cpp
        core/previewpage/to-pdf.cpp
        core/third_party/qpdflib/qpdfwidget.cpp
        core/third_party/qpdflib/pdfjsbridge.cpp
        core/third_party/qpdflib/pdfview.qrc
    )
endif()

add_library(ukui-previewhub SHARED ${UKUI_PREVIEWHUB_SOURCES})
add_dependencies(ukui-previewhub ukui-previewhub-translations)

target_include_directories(ukui-previewhub
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ukui-previewhub>
    PRIVATE
        "${CORE}"
        "${CORE}/third_party/qpdflib"
)

target_compile_definitions(ukui-previewhub
    PRIVATE
        USE_KSyntaxHighlighting
        QT_DEPRECATED_WARNINGS
        UKUIPREVIEWHUB_LIBRARY
)

if(UKUI_PREVIEWHUB_ENABLE_WEBENGINE_PREVIEW)
    target_compile_definitions(ukui-previewhub
        PRIVATE
            QPDFLIB_BUILD
            USE_WEBENGINE_PREVIEW
    )
endif()

target_link_libraries(ukui-previewhub
    PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Multimedia
        Qt5::MultimediaWidgets
        KF5::SyntaxHighlighting
)

if(UKUI_PREVIEWHUB_ENABLE_WEBENGINE_PREVIEW)
    target_link_libraries(ukui-previewhub
        PRIVATE
            Qt5::WebEngine
            Qt5::WebEngineCore
            Qt5::WebEngineWidgets
            Qt5::WebChannel
    )
endif()

set_target_properties(ukui-previewhub PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "ukui-previewhub"
)

install(TARGETS ukui-previewhub
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ukui_previewhub_export.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ukui_previewhub_api.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ukui-previewhub
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ukui-previewhub.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

install(FILES ${UKUI_PREVIEWHUB_TRANSLATION_QMS}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/ukui-previewhub/translations
)
