cmake_minimum_required(VERSION 3.14)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(UKUI_FILE_METADATA_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}")

include(GNUInstallDirs)

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Xml LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Xml LinguistTools REQUIRED)

set(PLUGIN_INSTALL_DIR "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/ukui-file-metadata/plugins")
set(EXTRACTOR_PLUGIN_IFACE_VERSION "1.0.0")
configure_file (ukui-file-metadata-config.h.in ukui-file-metadata-config.h @ONLY)

set(TS_FILES ukui-file-metadata_zh_CN.ts)

set(HEADERS
        ukui-file-metadata_global.h
        extractor-manager.h
        extractor-plugin.h
        extraction-result.h
        embedded-image-data.h
        types.h
        type-info.h
        properties.h
        extractor.h
        mime-utils.h
        simple-extraction-result.h
        property-info.h
        thumbnail.h
        ocr-utils.h
        zip-reader.h
        bookmark.h
        bookmarks-manager.h
        )

set(ukui-file-metadata_SRCS
        extraction-result.cpp
        embedded-image-data.cpp
        type-info.cpp
        extractor.cpp
        extractor-plugin.cpp
        extractor-manager.cpp
        mime-utils.cpp
        simple-extraction-result.cpp
        property-info.cpp
        #    format-strings.cpp
        thumbnail.cpp
        thumbnail.h
        thumbnail-utils.cpp
        thumbnail-utils.h
        ocr-utils.cpp
        ocr-utils.h
        zip-reader.cpp
        zip-reader.h
        bookmark.cpp
        bookmark.h
        bookmarks-manager.cpp
        bookmarks-manager.h)

add_library(ukui-file-metadata SHARED
    ${ukui-file-metadata_SRCS}
)

target_link_libraries(ukui-file-metadata  PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Xml
        Qt${QT_VERSION_MAJOR}::Widgets
        PRIVATE
        PkgConfig::TESSERACT
        PkgConfig::LEPT
        PkgConfig::MINIZIP
        ZLIB::ZLIB)

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/ukui-file-metadata")
set(HEADERS_INSTALL_DIR /usr/include/ukui-file-metadata)
set(PC_INSTALL_DIR "/usr/lib/pkgconfig")

target_include_directories(ukui-file-metadata PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${HEADERS_INSTALL_DIR}>)
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/ukui-file-metadata.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-file-metadata.pc
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR}
)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-file-metadata-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-file-metadata-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR})
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-file-metadata-config-version.cmake
        VERSION ${UKUI_FILE_METADATA_VERSION}
        COMPATIBILITY SameMajorVersion
)

set_target_properties(ukui-file-metadata  PROPERTIES
        VERSION ${UKUI_FILE_METADATA_VERSION}
        SOVERSION ${VERSION_MAJOR}
        OUTPUT_NAME ukui-file-metadata
        )

export(TARGETS ukui-file-metadata
        FILE "${CMAKE_CURRENT_BINARY_DIR}/ukui-file-metadata-targets.cmake")

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

add_subdirectory(extractors)

install(TARGETS ukui-file-metadata
        EXPORT ukui-file-metadata
        PUBLIC_HEADER DESTINATION ${HEADERS_INSTALL_DIR}
        LIBRARY DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
        )
install(EXPORT ukui-file-metadata
        FILE ukui-file-metadata-targets.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
install(FILES ${HEADERS} DESTINATION "/usr/include/ukui-file-metadata")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-file-metadata.pc DESTINATION ${PC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-file-metadata-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-file-metadata-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
