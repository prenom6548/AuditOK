cmake_minimum_required(VERSION 3.16)

project(ukui-menu-starter VERSION 4.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# TODO: 将依赖倒入的操作放到顶级CMakeList文件中
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Widgets DBus LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Widgets DBus LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(ukui-quick COMPONENTS platform REQUIRED)
find_package(ukui-search)

set(EXTERNAL_LIBS "")
if (QT_VERSION_MAJOR EQUAL "5")
    set(PC_PKGS gsettings-qt kysdk-systime)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(PC_PKGS gsettings-qt6 kysdk-systime)
endif()

foreach(external_lib IN ITEMS ${PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()

set(PROJECT_SOURCES
        plugin/start-menu-plugin.h plugin/start-menu-plugin.cpp
        plugin/start-menu-button.h plugin/start-menu-button.cpp
)

file(GLOB TS_FILES i18n/*.ts)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${QM_FILES})

else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES} ${QM_FILES})
endif()

target_compile_definitions(${PROJECT_NAME}
        PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        ukui-quick::platform
        ukui-search
)

install(DIRECTORY "widget/" DESTINATION "/usr/share/ukui/widgets/org.ukui.menu.starter")
install(FILES ${QM_FILES} DESTINATION "/usr/share/ukui/widgets/org.ukui.menu.starter/i18n")
install(FILES "plugin/qmldir" DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/menu/starter")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION  "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/ukui/menu/starter")
install(FILES ukui-menu-starter-v11.json DESTINATION "/opt/system/conf/ukui-security-support/")

if(BUILD_TEST)
    add_subdirectory(autotest)
endif()
