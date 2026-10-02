find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(KYSDK_UKUIWINDOWHELPER REQUIRED kysdk-ukuiwindowhelper)
if (KYSDK_UKUIWINDOWHELPER_FOUND)
    include_directories(${KYSDK_UKUIWINDOWHELPER_INCLUDE_DIRS})
endif()

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTOUIC ON) 
set(CMAKE_AUTORCC ON)

qt5_add_resources(VirtualKeyboard_SRC
	src/keyboard.qrc)

qt5_wrap_cpp(VirtualKeyboard_SRC
        src/fakekeyboard.h
        src/qxkbcommon_p.h
        ../common/capslockstatehelper.h
    )

set(VirtualKeyboard_SRC
        ${VirtualKeyboard_SRC}
        src/charsmorewidget.cpp
        src/charswidget.cpp
        src/dragwidget.cpp
        src/kbbutton.cpp
        src/kbtitle.cpp
        src/letterswidget.cpp
        src/numberswidget.cpp
        src/virtualkeyboardwidget.cpp
        src/x11keyboard.cpp
        src/qtkeyboard.cpp
        src/qxkbcommon.cpp
        src/qxkbcommon_3rdparty.cpp
        ../common/capslockstatehelper.cpp
        )

include_directories(
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
        )

add_library(VirtualKeyboard STATIC ${VirtualKeyboard_SRC})
target_link_libraries(VirtualKeyboard
    Qt5::Core Qt5::Widgets
    Qt5::X11Extras
    KF5::WaylandClient
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    ${KYSDK_UKUIWINDOWHELPER_LIBRARIES}
    xkbcommon)
