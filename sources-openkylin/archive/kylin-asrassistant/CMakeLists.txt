cmake_minimum_required(VERSION 3.5)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

project(kylin-asrassistant)

SET(CMAKE_CXX_FLAGS "-Wl,-z,now")

add_subdirectory(speechrecognitionclient)
add_subdirectory(speechinput)
add_subdirectory(src)

#install(FILES ${TOP_DIR}/kylin-asrassistant.desktop DESTINATION /usr/share/applications/)
install(FILES ${TOP_DIR}/kylin-asrassistant.desktop DESTINATION /etc/xdg/autostart/)
install(FILES  ${TOP_DIR}/config/speechrecognitionusr.conf DESTINATION /usr/share/kylin-asrassistant/)