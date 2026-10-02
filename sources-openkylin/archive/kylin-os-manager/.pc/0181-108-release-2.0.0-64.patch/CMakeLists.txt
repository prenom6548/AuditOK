cmake_minimum_required(VERSION 3.16)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

project(kylin-os-manager)

SET(CMAKE_CXX_FLAGS "-Wl,-z,now")

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_definitions(-DDEBUG_MODE)
endif()

add_subdirectory(3rdparty)
add_subdirectory(kom)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(daemons)
add_subdirectory(treasure-chest)

install(FILES ${TOP_DIR}/data/kylin-os-manager.desktop DESTINATION /usr/share/applications/)
install(FILES ${TOP_DIR}/data/kylin-os-manager-service-support.desktop DESTINATION /usr/share/applications/)
install(FILES ${TOP_DIR}/data/config/kylin-os-manager-plugin.ini DESTINATION /etc/kylin-os-manager/)
install(FILES ${TOP_DIR}/data/kylin-service-support DESTINATION /usr/share/kylin-os-manager/)
install(FILES ${TOP_DIR}/data/config/kylin-os-manager.conf DESTINATION /etc/ld.so.conf.d/)
install(FILES ${TOP_DIR}/data/kylin-os-manager-device-manager.desktop DESTINATION /usr/share/applications/)
