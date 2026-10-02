file(GLOB SESSION_FILES_IN ukui.desktop)
file(GLOB WAYLAND_SESSION_FILES_IN ukui-wayland.desktop)

install(FILES ${SESSION_FILES_IN} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/xsessions")
install(FILES ${WAYLAND_SESSION_FILES_IN} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/wayland-sessions")

install(FILES ukui-session-wayland DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

install(FILES org.ukui.session.gschema.xml DESTINATION "${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas")

install(FILES logout.wav weakup.wav startup.wav shutdown.wav DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ukui/ukui-session-manager")

install(DIRECTORY images DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ukui/ukui-session-manager")

install(DIRECTORY config DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ukui/ukui-session-manager")

install(FILES ukui-session.target DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/systemd/user")

install(FILES ukui-session-tools.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")

install(FILES org.gnome.SessionManager.policy DESTINATION "${CMAKE_INSTALL_PREFIX}/share/polkit-1/actions")

install(FILES com.kylin.enable-hibernate.rules DESTINATION "${CMAKE_INSTALL_PREFIX}/share/polkit-1/rules.d")

install(PROGRAMS clean_all_jobs.py DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/ukui/ukui-session/logout")

install(FILES com.ubuntu.enable-hibernate.pkla DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/polkit-1/localauthority/50-local.d")

install(FILES 99ukui-environment DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/X11/Xsession.d")

install(FILES configSpecialMode DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/ukui/ukui-session/")

