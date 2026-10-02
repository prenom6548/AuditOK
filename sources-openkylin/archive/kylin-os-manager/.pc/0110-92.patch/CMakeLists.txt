cmake_minimum_required(VERSION 3.16)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

project(kylin-os-manager)

SET(CMAKE_CXX_FLAGS "-Wl,-z,now")

add_link_options("-Wl,-rpath,/usr/lib/kylin-os-manager/common/")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,/usr/lib/kylin-os-manager/common/")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,/usr/lib/kylin-os-manager/common/")

add_subdirectory(3rdparty)
add_subdirectory(kom)
add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(daemons)
add_subdirectory(treasure-chest)

install(FILES ${TOP_DIR}/data/kylin-os-manager.desktop DESTINATION /usr/share/applications/)
install(FILES ${TOP_DIR}/data/kylin-service-support.desktop DESTINATION /usr/share/applications/)
install(FILES ${TOP_DIR}/data/kylin-service-support DESTINATION /usr/bin/)
install(FILES ${TOP_DIR}/data/config/kylin-os-manager.conf DESTINATION /etc/ld.so.conf.d/)
install(FILES ${TOP_DIR}/data/config/kylin-os-manager-plugin.conf DESTINATION /etc/kylin-os-manager/)
install(FILES ${TOP_DIR}/data/config/kylin-os-manager-box.conf DESTINATION /etc/kylin-os-manager/)
install(FILES ${TOP_DIR}/data/config/kylin-os-manager-plugin.ini DESTINATION /etc/kylin-os-manager/)
install(FILES ${TOP_DIR}/data/kylin-os-manager-device-manager.desktop DESTINATION /usr/share/applications/)
