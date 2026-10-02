add_subdirectory(tools)

set(UkuiWaylandProtocols_SRCS)

ukui_add_wayland_client_protocol(UkuiWaylandProtocols_SRCS
    PROTOCOL "${PROJECT_WAYLAND_PROTOCOLS_DIR}/xdg-shell.xml"
    BASENAME xdg-shell
)

ukui_add_wayland_client_protocol(UkuiWaylandProtocols_SRCS
    PROTOCOL "${PROJECT_WAYLAND_PROTOCOLS_DIR}/xdg-decoration-unstable-v1.xml"
    BASENAME xdg-decoration-unstable-v1
)

ukui_add_wayland_client_protocol(UkuiWaylandProtocols_SRCS
    PROTOCOL "${PROJECT_WAYLAND_PROTOCOLS_DIR}/xdg-dialog-v1.xml"
    BASENAME xdg-dialog-v1
)

ukui_add_wayland_client_protocol(UkuiWaylandProtocols_SRCS
    PROTOCOL "${PROJECT_WAYLAND_PROTOCOLS_DIR}/ukui-shell.xml"
    BASENAME ukui-shell
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(ukui-shell SHARED
    logging.cpp
    main.cpp
    ukuishell.cpp
    ukuishellintegration.cpp
    ukuishellsurface.cpp
    xdgdecorationv1.cpp
    xdgdialogv1.cpp
    ${UkuiWaylandProtocols_SRCS}
)

target_link_libraries(ukui-shell PRIVATE
    Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
    ${WAYLAND_LIBRARIES}
)

if (QT_VERSION_MAJOR EQUAL "5")
    target_link_libraries(ukui-shell PRIVATE
        Qt::XkbCommonSupportPrivate
    )
endif()

set_target_properties(ukui-shell
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/wayland-shell-integration
)

install(TARGETS ukui-shell
    LIBRARY DESTINATION ${QT_PLUGINS_INSTALL_DIR}/wayland-shell-integration/
)
