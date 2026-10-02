message(STATUS "Building ukui-voice-wing window")

set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Gui Qml Quick QuickControls2 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(Gsettings REQUIRED IMPORTED_TARGET gsettings-qt)
pkg_check_modules(KysdkUkuiWindowHelper REQUIRED IMPORTED_TARGET kysdk-ukuiwindowhelper)

add_library(${PROJECT_NAME}-window STATIC
    voiceInputWindow.cpp
    voiceInputWindow.h
    microphoneMuteDialog.cpp
    microphoneMuteDialog.h
    themeIconProvider.cpp
    themeIconProvider.h
    voiceInputWindow.qrc
)

set_target_properties(${PROJECT_NAME}-window PROPERTIES
    AUTOMOC ON
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${PROJECT_NAME}-window
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(${PROJECT_NAME}-window
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Qml
        Qt5::Quick
        Qt5::QuickControls2
        PkgConfig::Gsettings
        PkgConfig::KysdkUkuiWindowHelper
        ${PROJECT_NAME}-log
)
