cmake_minimum_required(VERSION 3.16)

project(framework)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# 依赖Widgets是为了使用QAction,在高版本qt中，QAction已挪到Gui模块，不需要依赖Widgets
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Widgets REQUIRED)

set(PROJECT_SOURCES
        widget/island.cpp widget/island.h
        widget/widget.cpp widget/widget.h
        widget/widget-loader.cpp widget/widget-loader.h
        widget/widget-metadata.cpp widget/widget-metadata.h
        widget/widget-content.cpp widget/widget-content.h
        widget/widget-container.cpp widget/widget-container.h
        widget/widget-interface.h
        widget-ui/widget-bridge.cpp widget-ui/widget-bridge.h
        widget-ui/widget-feature-provider.h
        widget-ui/widget-item.cpp widget-ui/widget-item.h
        widget-ui/widget-item-engine.cpp widget-ui/widget-item-engine.h
        widget-ui/widget-item-context.cpp widget-ui/widget-item-context.h
        widget-ui/widget-item-attached.cpp widget-ui/widget-item-attached.h
        widget-ui/widget-container-item.cpp widget-ui/widget-container-item.h
        config/config.h config/config.cpp
        config/ini-config.h config/ini-config.cpp
        config/config-loader.h config/config-loader.cpp
        config/config-property-map.cpp config/config-property-map.h
        view/island-view.cpp view/island-view.h
)
set(HEADERS
        widget/island.h
        widget/widget.h
        widget/widget-loader.h
        widget/widget-metadata.h
        widget/widget-container.h
        widget/widget-content.h
        widget/widget-interface.h
        widget-ui/widget-bridge.h
        widget-ui/widget-feature-provider.h
        widget-ui/widget-item.h
        widget-ui/widget-container-item.h
        config/config.h
        config/ini-config.h
        config/config-loader.h
        config/config-property-map.h
        view/island-view.h
)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})
add_library(${ROOT_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
include_directories(config)
include_directories(widget)
include_directories(widget-ui)
include_directories(../core)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ukui-quick::core
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
)

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/ukui-quick-framework")
set(HEADERS_INSTALL_DIR "/usr/include/ukui-quick/framework")
set(PC_INSTALL_DIR "/usr/lib/pkgconfig")
target_include_directories(${PROJECT_NAME}
        INTERFACE
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
ukui_qdoc_add_qch(ukui-quick-framework-doc
        NAME ukui-quick-framework
        DOC_NAME ukui-quick-framework
        VERSION ${UKUI_QUICK_VERSION}
        NAMESPACE org.ukui
        QCH_INSTALL_DESTINATION "/usr/share/qt${QT_VERSION_MAJOR}/doc"
        SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        HEADER_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        CONFIG_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-extend/modules/QDocConfig.qdocconf.in"
        INCLUDE_DIRS ${include_dirs}
        DEPEND_QCHS qtcore qtqml qtquick
        STYLE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-extend/style/default.css"
        IMAGE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/doc/images"
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-quick-framework-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-framework-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR}
)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-framework-config-version.cmake
        VERSION ${UKUI_QUICK_VERSION}
        COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-quick-framework.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-framework.pc"
        INSTALL_DESTINATION  ${PC_INSTALL_DIR}
)
set_target_properties(${PROJECT_NAME}  PROPERTIES
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
        FILE ukui-quick-framework-targets.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}
        NAMESPACE ${ROOT_PROJECT_NAME}::
)

install(FILES ${HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-framework.pc DESTINATION ${PC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-framework-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-framework-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
if(BUILD_TEST AND BUILD_INTERACTIVE_TESTS)
    add_subdirectory(test)
endif ()
if(BUILD_TEST)
    add_subdirectory(autotest)
endif ()
