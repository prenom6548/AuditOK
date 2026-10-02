cmake_minimum_required(VERSION 3.16)

project(treasure-chest)

add_subdirectory(replacement)
add_subdirectory(file-smash)
add_subdirectory(network-bandwidth-test)
add_subdirectory(network-route-track)
add_subdirectory(network-iputils-arping)
add_subdirectory(network-telnet)
add_subdirectory(system-cleanup)
add_subdirectory(tcpdump)
add_subdirectory(source-update)
add_subdirectory(win-data-migration)

install(FILES 3rdparty/kylin-baobab.conf DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/plugins/)
install(FILES 3rdparty/kylin-d-feet.conf DESTINATION /usr/share/kylin-os-manager/plugins/tool-box/plugins/)

# 安装独立的用户手册
install(DIRECTORY 3rdparty/user_manual/d-feet/ DESTINATION /usr/share/kylin-user-guide/data/guide/d-feet/)
install(DIRECTORY 3rdparty/user_manual/baobab/ DESTINATION /usr/share/kylin-user-guide/data/guide/baobab/)
