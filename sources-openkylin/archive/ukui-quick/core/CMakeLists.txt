cmake_minimum_required(VERSION 3.14)
project(core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Qml REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Qml REQUIRED)

set(UKUI_QUICK_CORE_SRCS
        margin.cpp
        types.cpp
        shared-engine-view.cpp
        shared-qml-engine.cpp
        shared-engine-component.cpp
)
set(HEADERS
        margin.h
        types.h
        shared-engine-view.h
        shared-qml-engine.h
        shared-engine-component.h
)
add_library(${PROJECT_NAME} SHARED
        ${UKUI_QUICK_CORE_SRCS}
)
add_library(ukui-quick::core ALIAS ${PROJECT_NAME})
add_library(${ROOT_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
)

set(HEADERS_INSTALL_DIR /usr/include/ukui-quick/core)
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:${HEADERS_INSTALL_DIR}>
)

#文档
include(UkuiQDocAddQch)
set(include_dirs
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtCore"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtQml"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtQuick"
)
#文档
ukui_qdoc_add_qch(ukui-quick-core-doc
        NAME ukui-quick-core
        DOC_NAME ukui-quick-core
        VERSION ${UKUI_QUICK_VERSION}
        NAMESPACE org.ukui
        QCH_INSTALL_DESTINATION "/usr/share/qt${QT_VERSION_MAJOR}/doc"
        SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        HEADER_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        CONFIG_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-extend/modules/QDocConfig.qdocconf.in"
        INCLUDE_DIRS ${include_dirs}
        DEPEND_QCHS qtcore qtqml qtquick
        STYLE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-extend/style/default.css"
)

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/ukui-quick-core")
set(PC_INSTALL_DIR "/usr/lib/pkgconfig")

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-quick-core-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-core-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR})
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-core-config-version.cmake
        VERSION ${UKUI_QUICK_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-quick-core.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-core.pc"
        INSTALL_DESTINATION  ${PC_INSTALL_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${UKUI_QUICK_VERSION}
        SOVERSION ${VERSION_MAJOR}
        OUTPUT_NAME ${ROOT_PROJECT_NAME}-${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${HEADERS_INSTALL_DIR}
        LIBRARY DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
)
install(EXPORT ${PROJECT_NAME}
        FILE ukui-quick-core-targets.cmake
        NAMESPACE ${ROOT_PROJECT_NAME}::
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

install(FILES ${HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-core.pc DESTINATION ${PC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-core-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-core-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

if(BUILD_TEST)
    add_subdirectory(autotest)
endif()
