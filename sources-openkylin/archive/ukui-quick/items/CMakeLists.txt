cmake_minimum_required(VERSION 3.14)
project(ukui-quick-items-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find QT modules
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Qml Quick Widgets Gui LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Qml Quick Widgets Gui LinguistTools REQUIRED)

if (QT_VERSION_MAJOR EQUAL "5")
    set(KF_VERSION_MAJOR "5")
    find_package(KF5WindowSystem REQUIRED)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(KF_VERSION_MAJOR "6")
    find_package(Qt6 REQUIRED NO_MODULE COMPONENTS ShaderTools)
    find_package(KF6WindowSystem REQUIRED)
endif()

find_package(PkgConfig REQUIRED)

set(EXTERNAL_LIBS "")

if (QT_VERSION_MAJOR EQUAL 5)
    set(PC_PKGS Qt5Xdg)
elseif (QT_VERSION_MAJOR EQUAL 6)
    set(PC_PKGS Qt6Xdg)
endif ()

foreach(PC_LIB IN ITEMS ${PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

pkg_check_modules(THORVG REQUIRED IMPORTED_TARGET thorvg-1)
list(APPEND EXTERNAL_LIBS PkgConfig::THORVG)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(UKUI_QUICK_CORE_ITEMS_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(PLUGIN_INSTALL_ROOT_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/qml/org/ukui/quick/items")
set(PLUGIN_INSTALL_TRANSLATIONS_PATH "${PLUGIN_INSTALL_ROOT_PATH}/translations/")
add_compile_definitions(PLUGIN_INSTALL_TRANSLATIONS_PATH="${PLUGIN_INSTALL_TRANSLATIONS_PATH}")
include_directories(views)
# items模块的功能不需要导出到include目录，仅使用qml的import语句进行导入使用
set(ITEMS_LOTTIE_RUNTIME_SRCS
        lottie-internal-visibility_p.h
        lottie-runtime-diagnostics_p.h
        lottie-runtime-service.cpp lottie-runtime-service_p.h
        lottie-runtime-raster-worker.cpp lottie-runtime-raster-worker_p.h
        lottie-runtime-frame-scheduler-diagnostics_p.h
        lottie-runtime-frame-scheduler.cpp lottie-runtime-frame-scheduler_p.h
        lottie-renderer.cpp lottie-renderer.h
)
set (ITEMS_PLUGIN_SRCS
        ukui-quick-items-plugin.cpp ukui-quick-items-plugin.h
        menu.cpp menu.h
        menu-item.cpp menu-item.h
        theme-icon.cpp theme-icon.h
        tooltip-proxy.cpp tooltip-proxy.h
        icon-provider.cpp icon-provider.h
        icon.cpp icon.h
        lottie-attached.cpp lottie-attached.h
        lottie-monotonic-clock_p.h
        lottie-host-renderability.cpp lottie-host-renderability_p.h
        lottie-item.cpp lottie-item.h
        icon-heuristics.cpp icon-heuristics.h
        icon-helper.cpp icon-helper.h
        color-mixer.cpp color-mixer.h
        action-extension.cpp action-extension.h
        gradient-border-item.cpp gradient-border-item.h
        line-edit-context-menu.cpp line-edit-context-menu.h
        views/content-window.cpp views/content-window.h
        views/tooltip.cpp views/tooltip.h
        views/ukui-style-window.cpp views/ukui-style-window.h
        views/tooltip-dialog.cpp views/tooltip-dialog.h
        window-blur-behind.cpp window-blur-behind.h
        shadowed-rectangle.cpp shadowed-rectangle.h
        shadowed-texture.cpp shadowed-texture.h
        scenegraph/painted-rectangle-item.cpp scenegraph/painted-rectangle-item.h
        scenegraph/shadowed-rectangle-node.cpp scenegraph/shadowed-rectangle-node.h
        scenegraph/shadowed-border-rectangle-material.cpp scenegraph/shadowed-border-rectangle-material.h
        scenegraph/shadowed-rectangle-material.cpp scenegraph/shadowed-rectangle-material.h
        scenegraph/shadowed-texture-node.cpp scenegraph/shadowed-texture-node.h
        scenegraph/shadowed-border-texture-material.cpp scenegraph/shadowed-border-texture-material.h
        scenegraph/shadowed-texture-material.cpp scenegraph/shadowed-texture-material.h
)
set(QRC_FILES qml/qml.qrc)
if (QT_VERSION_MAJOR EQUAL "5")
    list(APPEND QRC_FILES shaders/shaders.qrc)
endif()

file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/qml ${TS_FILES} OPTIONS -no-ui-lines)
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/qml ${TS_FILES} OPTIONS -no-ui-lines)
endif()

add_library(${PROJECT_NAME} SHARED ${ITEMS_PLUGIN_SRCS} ${QRC_FILES} ${QM_FILES})

function(configure_items_plugin target lottie_core_target)
    target_include_directories(${target} PRIVATE ../core)
    target_include_directories(${target} PRIVATE ../platform)
    target_include_directories(${target} PRIVATE ../framework)
    target_include_directories(${target} PRIVATE scenegraph)
    target_compile_definitions(${target} PRIVATE
            PLUGIN_IMPORT_URI="org.ukui.quick.items"
            PLUGIN_VERSION_MAJOR=${VERSION_MAJOR}
            PLUGIN_VERSION_MINOR=${VERSION_MINOR}
    )
    target_link_libraries(${target}
            PRIVATE
            Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Qml
            Qt${QT_VERSION_MAJOR}::Quick
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Gui
            Qt::GuiPrivate
            KF${KF_VERSION_MAJOR}::WindowSystem
            ukui-quick::core
            ukui-quick::platform
            ukui-quick::framework
            ${lottie_core_target}
            ${EXTERNAL_LIBS}
    )
endfunction()

add_library(ukui-quick-items-lottie-runtime OBJECT
    ${ITEMS_LOTTIE_RUNTIME_SRCS}
)
configure_items_plugin(ukui-quick-items-lottie-runtime ukui-quick-lottie-core)
set_target_properties(ukui-quick-items-lottie-runtime PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)
target_sources(${PROJECT_NAME} PRIVATE
    $<TARGET_OBJECTS:ukui-quick-items-lottie-runtime>
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VISIBILITY_INLINES_HIDDEN YES
)

if(BUILD_TEST)
    # BUILD_TEST must leave the production plugin unchanged. Tests that need
    # private bottom-dependency seams link this additional, never-installed
    # plugin instead of injecting test code into the production target.
    add_library(ukui-quick-items-test-plugin SHARED
        ${ITEMS_PLUGIN_SRCS}
        ${QRC_FILES}
        ${QM_FILES}
    )
    add_library(ukui-quick-items-lottie-runtime-test OBJECT
        ${ITEMS_LOTTIE_RUNTIME_SRCS}
    )
    configure_items_plugin(ukui-quick-items-lottie-runtime-test
        ukui-quick-lottie-core-test)
    target_compile_definitions(ukui-quick-items-lottie-runtime-test PRIVATE
        UKUI_QUICK_LOTTIE_CORE_TEST_EXPORTS
        UKUI_QUICK_LOTTIE_INTERNAL_TEST_EXPORTS
        UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM
    )
    set_target_properties(ukui-quick-items-lottie-runtime-test PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
    target_sources(ukui-quick-items-test-plugin PRIVATE
        $<TARGET_OBJECTS:ukui-quick-items-lottie-runtime-test>
    )
endif()

#for qt6
add_subdirectory(shaders6)

configure_items_plugin(${PROJECT_NAME} ukui-quick-lottie-core)

if(BUILD_TEST)
    configure_items_plugin(ukui-quick-items-test-plugin ukui-quick-lottie-core-test)
    target_compile_definitions(ukui-quick-items-test-plugin PRIVATE
        UKUI_QUICK_LOTTIE_CORE_TEST_EXPORTS
        UKUI_QUICK_LOTTIE_INTERNAL_TEST_EXPORTS
        UKUI_QUICK_LOTTIE_INTERNAL_TEST_SEAM
    )
endif()

#文档
include(UkuiQDocAddQch)
set(include_dirs
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtCore"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtQml"
        "/usr/include/${CMAKE_LIBRARY_ARCHITECTURE}/qt${QT_VERSION_MAJOR}/QtQuick"
)
#文档
ukui_qdoc_add_qch(ukui-quick-items-doc
        NAME ukui-quick-items
        DOC_NAME ukui-quick-items
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

install(DIRECTORY "qml" DESTINATION "${PLUGIN_INSTALL_ROOT_PATH}")
install(FILES qmldir DESTINATION "${PLUGIN_INSTALL_ROOT_PATH}")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION "${PLUGIN_INSTALL_ROOT_PATH}")
install(FILES ${QM_FILES} DESTINATION "${PLUGIN_INSTALL_TRANSLATIONS_PATH}")
install(FILES iconThemeBackgroundConfig.json DESTINATION "/usr/share/ukui/ukui-quick-items")

if(BUILD_TEST)
    add_subdirectory(autotest)
endif()
