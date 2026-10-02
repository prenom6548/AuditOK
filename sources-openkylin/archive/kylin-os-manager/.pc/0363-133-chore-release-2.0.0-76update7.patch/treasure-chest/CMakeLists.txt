cmake_minimum_required(VERSION 3.16)

project(treasure-chest)

add_subdirectory(file-smash)
add_subdirectory(network-bandwidth-test)
add_subdirectory(network-route-track)
add_subdirectory(network-iputils-arping)
add_subdirectory(network-telnet)
add_subdirectory(system-cleanup)
add_subdirectory(tcpdump)
add_subdirectory(source-update)

install(FILES 3rdparty/kylin-baobab.conf DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/plugins/)
install(FILES 3rdparty/kylin-d-feet.conf DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/plugins/)
