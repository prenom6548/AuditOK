execute_process(
	COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=system_bus_services_dir dbus-1
	OUTPUT_VARIABLE DBUS_SYSTEM_SERVICES_DIR
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=systemdsystemunitdir systemd
	OUTPUT_VARIABLE SYSTEMD_SYSTEM_UNIT_DIR
	OUTPUT_STRIP_TRAILING_WHITESPACE
	)

configure_file(
	miracle-wifid.service.cmake
	miracle-wifid.service
	)
install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/miracle-wifid.service
	DESTINATION ${SYSTEMD_SYSTEM_UNIT_DIR}/
	)

install(
	CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${SYSTEMD_SYSTEM_UNIT_DIR}/miracle-wifid.service \$ENV{DESTDIR}${SYSTEMD_SYSTEM_UNIT_DIR}/dbus-org.freedesktop.miracle.wifi.service)"
	)

INSTALL(
    PROGRAMS uibc-viewer
    DESTINATION bin
    )

INSTALL(
    PROGRAMS checkhardware.sh checkDaemonRunning.sh checkFrontendRunning.sh execstart.sh recover_wpas.sh
    DESTINATION bin
    )

INSTALL(
    FILES org.freedesktop.miracle.conf
    DESTINATION ${SYSCONFDIR}/dbus-1/system.d
    )

INSTALL(
    FILES miracle-agent.desktop
    DESTINATION ${SYSCONFDIR}/xdg/autostart
    )

if (EXISTS ${DATADIR})
    message("DATADIR is configed as " ${DATADIR})
else()
    SET(DATADIR "/usr/local/share")
endif()
message("DATADIR is finally configed as " ${DATADIR})

INSTALL(
    FILES miracle-wifid miracle-sinkctl miracle-wifictl
    DESTINATION ${DATADIR}/bash-completion/completions
    OPTIONAL
    )
