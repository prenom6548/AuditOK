cmake_minimum_required(VERSION 2.6)
project(ukui-screensaver)

## 更新翻译字段
file(GLOB TS_FILES ./i18n_ts/*.ts)
add_custom_command(
        OUTPUT ${TS_FILES}
        COMMAND lupdate src/ screensaver/ -ts ${TS_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
add_custom_target(
        i18n_ts
        DEPENDS ${TS_FILES}
        )

add_compile_options(-fPIC)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_PREFIX /usr)

set(Debug ON)
if(Debug)
    set(CMAKE_BUILD_TYPE "Debug")                                                                                                     
endif()

add_subdirectory(src)
#add_subdirectory(tests)
add_subdirectory(i18n_ts)
add_subdirectory(ukccplugins_i18n_ts)
add_subdirectory(data)

# 让主目标依赖翻译文件，这样才会执行i18n
add_dependencies(ukui-screensaver-dialog i18n ukccplugins_i18n)
