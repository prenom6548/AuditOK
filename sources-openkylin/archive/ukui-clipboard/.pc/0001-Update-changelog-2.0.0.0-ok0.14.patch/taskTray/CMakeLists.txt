set(taskTray_SRCS
	main.cpp
        clipboarddata.cpp
        clipboarddata.h
        clipboardtray.cpp
        clipboardtray.h
        clipboardmodel.cpp
        clipboardmodel.h
        source.qrc
        ../qml.qrc
)

add_executable(ukui-clipboard-tray ${taskTray_SRCS})

target_link_libraries(ukui-clipboard-tray
	Qt5::Widgets
	Qt5::Core
	Qt5::Gui
        Qt5::QuickWidgets
        qtsingleapplication
	-lukui-log4qt
)

install(TARGETS ukui-clipboard-tray DESTINATION bin)
install(FILES ./data/ukui-clipboard-tray.desktop DESTINATION /etc/xdg/autostart/)
