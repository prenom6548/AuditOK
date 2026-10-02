cmake_minimum_required(VERSION 3.16)
 
project(icon-engines)
 
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
set(QT_MIN_VERSION "5.12.0")
 
 
# find_package(Qt5Gui)
# message("icon-engines imported_location = ${imported_location}")
# set(LIB_PATH ${imported_location})
# message("icon-engines LIB_PATH = ${LIB_PATH}")
 
# get_filename_component(PARENT_PATH "${LIB_PATH}" PATH)
# message("icon-engines PARENT_PATH 1 = ${PARENT_PATH}")
# get_filename_component(PARENT_PATH "${PARENT_PATH}" PATH)
# message("icon-engines PARENT_PATH 2 = ${PARENT_PATH}")
# set(TARGET_PATH ${PARENT_PATH})
# message("icon-engines target_path = ${TARGET_PATH}")
 
 
#打印结果
#con engine imported_location = /usr/lib/x86_64-linux-gnu/qt5/plugins/platforms/libqxcb.so
#icon engine LIB_PATH = /usr/lib/x86_64-linux-gnu/qt5/plugins/platforms/libqxcb.so
#icon engine PARENT_PATH 1 = /usr/lib/x86_64-linux-gnu/qt5/plugins/platforms
#icon engine PARENT_PATH 2 = /usr/lib/x86_64-linux-gnu/qt5/plugins
#iconengine PARENT_PATH: /usr/lib/x86_64-linux-gnu/qt5/plugins
#icon engine target_path = /usr/lib/x86_64-linux-gnu/qt5/plugins
#icon-engines TARGET_PATH: /usr/lib/x86_64-linux-gnu/qt5/plugins
 
 
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Gui
    Svg
)
 
 
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})
 
# 设置源文件
set(SOURCES
    usvg-iconengine.cpp
    usvg-iconengine-plugin.cpp
	usvg-iconengine.h
    usvg-iconengine-plugin.h
)

set(DISTFILES
    svgenginemetadata.json
)
 
 
# 生成插件库
add_library(icon-engines MODULE ${SOURCES})
 
# 定义宏
#add_definitions(
#    -DQT5UKUIICONENGINE_LIBRARY
#    -DQT_DEPRECATED_WARNINGS
#    -DQT_MESSAGELOGCONTEXT
#)
 
# 指定IconEngineEnd在构建过程中所需链接的 库
target_link_libraries(icon-engines
    Qt5::Core
    Qt5::Gui
    Qt5::Svg
    Qt5::Widgets
)

if(UNIX)
    set(TARGET_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/plugins")

    MESSAGE("icon-engines TARGET_PATH: ${TARGET_PATH}")

    install(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PATH}/iconengines/)
endif()
 
 
