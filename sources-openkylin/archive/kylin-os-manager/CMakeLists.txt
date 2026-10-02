cmake_minimum_required(VERSION 3.16)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

project(kylin-os-manager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wl,-z,now -Wall -g")
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_definitions(-DDEBUG_MODE)
endif()

# v11
add_definitions(-DV11_ENVIRONMENT)
option(V11_OPTION "Enable v11 compilation options" ON)

add_subdirectory(3rdparty)
add_subdirectory(kom)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(daemons)
add_subdirectory(treasure-chest)

install(FILES ${TOP_DIR}/data/kylin-os-manager.desktop DESTINATION /usr/share/applications/)
install(FILES ${TOP_DIR}/data/config/kylin-os-manager-plugin.ini DESTINATION /etc/kylin-os-manager/)
install(FILES ${TOP_DIR}/data/kylin-service-support DESTINATION /usr/share/kylin-os-manager/)
install(FILES ${TOP_DIR}/data/config/kylin-os-manager.conf DESTINATION /etc/ld.so.conf.d/)
install(FILES ${TOP_DIR}/data/kylin-os-manager.json DESTINATION /etc/ukui/ukui-notification/default-app/)
