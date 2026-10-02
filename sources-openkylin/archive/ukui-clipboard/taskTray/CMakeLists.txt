set(taskTray_SRCS
	main.cpp
        clipboarddata.cpp
        clipboarddata.h
        clipboardtray.cpp
        clipboardtray.h
        clipboardmodel.cpp
        clipboardmodel.h
        waylandbackendThread.cpp
        waylandbackendThread.h
        source.qrc
)

pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
include_directories(${GSETTINGS_QT_INCLUDES})

set(EXTERNAL_LIBS "")
set(PC_PKGS kysdk-waylandhelper kysdk-qtwidgets kysdk-ukuiwindowhelper)
foreach(external_lib IN ITEMS ${PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client++)

function(add_wayland_protocols protocol)
    find_program(WAYLAND_SCANNER wayland-scanner++)
    if (NOT WAYLAND_SCANNER)
        message(FATAL_ERROR "wayland-scanner++ not found! You need to install waylandpp-dev libwayland-dev package.")
    endif()

    set(PROTOCOL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
    set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

    add_custom_command(
        OUTPUT ${OUTPUT_DIR}/${protocol}.cpp
        COMMAND ${WAYLAND_SCANNER} ${PROTOCOL_DIR}/${protocol}.xml ${OUTPUT_DIR}/${protocol}.h ${OUTPUT_DIR}/${protocol}.cpp
        DEPENDS ${PROTOCOL_DIR}/${protocol}.xml
    )
endfunction(add_wayland_protocols protocol)

add_wayland_protocols(wlr-data-control-unstable-v1)

add_executable(ukui-clipboard-tray
    wlr-data-control-unstable-v1.cpp
    ${taskTray_SRCS})

target_link_libraries(ukui-clipboard-tray
	Qt5::Widgets
	Qt5::Core
	Qt5::Gui
        Qt5::QuickWidgets
        qtsingleapplication
	-lukui-log4qt
        ${EXTERNAL_LIBS}
        ${GSETTINGS_QT_LIBRARIES}
        wayland-client++
)

target_include_directories(ukui-clipboard-tray
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS ukui-clipboard-tray DESTINATION bin)
install(FILES ./data/ukui-clipboard-tray.desktop DESTINATION /etc/xdg/autostart/)
