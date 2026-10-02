message(STATUS "Building ukui-voice-wing data")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/org.ukui.voice-wing.desktop.in" "${CMAKE_CURRENT_BINARY_DIR}/org.ukui.voice-wing.desktop" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.ukui.voice-wing.desktop" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/org.ukui.voice-wing.desktop" DESTINATION "${CMAKE_INSTALL_PREFIX}/../etc/xdg/autostart")

install(FILES org.ukui.voice-wing.gschema.xml
    DESTINATION "${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas"
)
