cmake_minimum_required(VERSION 3.16)
project(soft-freeze-shortcut)

find_package(QT NAMES Qt6 Qt5 COMPONENTS
    Core Widgets LinguistTools
    REQUIRED
    )
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Core Widgets LinguistTools
    REQUIRED
    )
find_package(PkgConfig REQUIRED)

set(EXTERNAL_LIBS "")
if (QT_VERSION_MAJOR EQUAL "5")
    set(PC_PKGS gsettings-qt)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(PC_PKGS gsettings-qt6)
endif()

foreach(PC_LIB IN ITEMS ${PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS ${${PC_LIB}_LIBRARIES})
    endif()
endforeach()
include_directories(../../interface)
include(../../cmake/UkuiShortCommon.cmake)

# 搜索全部源文件
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

# 查找全部翻译文件 并生成qm文件
file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR} "*.ts")
# 更新翻译并创建.qm文件
if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
endif()
# 生成动态链接库[.so]文件
add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES} ${QM_FILES})

set(TRANSLATION_FILE_DIR "${SHORTCUT_TRANSLATION_FILE_DIR}")
# 编译宏定义
target_compile_definitions(${PROJECT_NAME}
    PRIVATE TRANSLATION_FILE_DIR="${TRANSLATION_FILE_DIR}"
    )
# 链接外部库
target_link_libraries(${PROJECT_NAME} PRIVATE
        ukui-shortcut
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        ${EXTERNAL_LIBS}
    )

# 安装.so和翻译文件
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION "${PLUGIN_INSTALL_DIRS}")
install(FILES ${QM_FILES} DESTINATION "${TRANSLATION_FILE_DIR}")
