set(SOURCES
    ReadBuffer.cpp
    DisplayControl.cpp
    DisplayControlDecoder.cpp
    DisplayControlServer.cpp
    DisplayControlThread.cpp
    SocketStream.cpp
    UnixStream.cpp
    display_control_api.cpp
    kmre_socket.c
    kmre_display_control.c)
add_library(DisplayControl SHARED ${SOURCES})
add_dependencies(DisplayControl Headers)
target_link_libraries(DisplayControl
    displayControl_dec emugl_common)
install(TARGETS DisplayControl LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

