set(CMAKE_AUTOMOC on)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Test Gui Quick Widgets DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} CONFIG REQUIRED COMPONENTS Test Gui Quick Widgets DBus)
find_package(PkgConfig REQUIRED)

if (QT_VERSION_MAJOR EQUAL "5")
    set(AUTOTEST_GSETTINGS_QT gsettings-qt)
elseif (QT_VERSION_MAJOR EQUAL "6")
    set(AUTOTEST_GSETTINGS_QT gsettings-qt6)
endif()

pkg_check_modules(${AUTOTEST_GSETTINGS_QT} REQUIRED IMPORTED_TARGET ${AUTOTEST_GSETTINGS_QT})

add_executable(startMenuButtonApiTest
        start-menu-button-api-test.cpp
        ../plugin/start-menu-button.cpp)

target_include_directories(startMenuButtonApiTest PRIVATE
        ..
)

target_link_libraries(startMenuButtonApiTest PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        PkgConfig::${AUTOTEST_GSETTINGS_QT}
        ukui-quick::platform
        ukui-search
)

add_test(NAME startMenuButtonApiTest COMMAND $<TARGET_FILE:startMenuButtonApiTest>)
set_tests_properties(startMenuButtonApiTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;HOME=/tmp;XDG_RUNTIME_DIR=/tmp")
