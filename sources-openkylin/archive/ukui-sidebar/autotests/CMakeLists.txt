set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Test DBus Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Test DBus Widgets REQUIRED)

include_directories(../)

add_executable(screenMonitorTest
    ../src/main/screen-monitor.cpp
    src/main/screen-monitor-test.cpp)

set(EXTERNAL_LIBS "")
set(PC_PKGS gsettings-qt)

foreach(lib IN ITEMS ${PC_PKGS})
    pkg_check_modules(${lib} REQUIRED ${lib})
    if(${${lib}_FOUND})
        include_directories(${${lib}_INCLUDE_DIRS})
        link_directories(${${lib}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS ${${lib}_LIBRARIES})
    endif()
endforeach()

target_link_libraries(screenMonitorTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        ${EXTERNAL_LIBS}
)
add_test(NAME screenMonitorTest COMMAND $<TARGET_FILE:screenMonitorTest>)

# handGestureHelperTest
add_executable(handGestureHelperTest
    ../src/main/hand-gesture-helper.cpp
    ../src/main/screen-monitor.cpp
    src/main/hand-gesture-helper-test.cpp)
target_link_libraries(handGestureHelperTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        ${EXTERNAL_LIBS}
)
add_test(NAME handGestureHelperTest COMMAND $<TARGET_FILE:handGestureHelperTest>)

# appManagerTest
add_executable(appManagerTest
    ../src/main/app-manager.cpp
    src/main/app-manager-test.cpp)
target_link_libraries(appManagerTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        ${EXTERNAL_LIBS}
)
add_test(NAME appManagerTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:appManagerTest>)

# accountInformationTest
find_package(ukui-quick COMPONENTS platform REQUIRED)
add_executable(accountInformationTest
    ../src/main/account-information.cpp
    ../src/main/app-manager.cpp
    src/main/stubs/dbus-connector.h
    ukui-shortcut/stubs/ukui-quick/platform/dbus-connector.h
    src/main/account-information-test.cpp)
target_include_directories(accountInformationTest BEFORE PRIVATE
    src/main/stubs
)
target_link_libraries(accountInformationTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        ${EXTERNAL_LIBS}
)
add_test(NAME accountInformationTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:accountInformationTest>)

# globalSettingsTest
add_executable(globalSettingsTest
    ../src/main/global-settings.cpp
    src/main/global-settings-test.cpp)
pkg_check_modules(GIO REQUIRED gio-2.0)
target_include_directories(globalSettingsTest PRIVATE ${GIO_INCLUDE_DIRS})
target_link_libraries(globalSettingsTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        ${EXTERNAL_LIBS}
        ${GIO_LIBRARIES}
)
add_test(NAME globalSettingsTest COMMAND $<TARGET_FILE:globalSettingsTest>)

# widgetDelegateTest
add_executable(widgetDelegateTest
    ../src/model/widget-delegate.cpp
    src/model/stubs/shared-engine-component.h
    src/model/stubs/widget-container.h
    src/model/stubs/widget-container-item.h
    src/model/stubs/widget-delegate-stubs.cpp
    src/model/stubs/widget-loader.h
    src/model/stubs/widget-metadata.h
    src/model/widget-delegate-test.cpp)
target_include_directories(widgetDelegateTest BEFORE PRIVATE
    src/model/stubs
)
target_link_libraries(widgetDelegateTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME widgetDelegateTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:widgetDelegateTest>)

# ukuiColumnTest
add_executable(ukuiColumnTest
    ../src/layout/ukui-column.cpp
    src/layout/ukui-column-test.cpp)
if (QT_VERSION_MAJOR EQUAL 5)
    target_include_directories(ukuiColumnTest PRIVATE ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
endif()
target_link_libraries(ukuiColumnTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::QmlPrivate
        Qt${QT_VERSION_MAJOR}::QuickPrivate
)
add_test(NAME ukuiColumnTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:ukuiColumnTest>)

# shortcutModelTest
find_package(ukui-quick COMPONENTS core REQUIRED)
set(UKUI_SHORTCUT_LOCAL_INCLUDE_DIRS
    ../ukui-shortcut
    ../ukui-shortcut/interface
)
add_executable(shortcutModelTest
    ../src/model/shortcut-model.cpp
    ../src/model/shortcut-item.cpp
    ../src/model/shortcut-filter-model.cpp
    ../src/model/edit-shortcut-model.cpp
    ../src/model/shortcut-config.cpp
    ../src/main/global-settings.cpp
    src/model/shortcut-model-test.cpp)
target_include_directories(shortcutModelTest PRIVATE
    ${UKUI_SHORTCUT_LOCAL_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)
target_link_libraries(shortcutModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        ukui-quick::platform
        ukui-quick::framework
        ukui-quick::core
        ${EXTERNAL_LIBS}
        ${GIO_LIBRARIES}
        ukui-shortcut
)
add_test(NAME shortcutModelTest COMMAND $<TARGET_FILE:shortcutModelTest>)

# editShortcutModelTest
add_executable(editShortcutModelTest
    ../src/model/shortcut-model.cpp
    ../src/model/shortcut-item.cpp
    ../src/model/shortcut-filter-model.cpp
    ../src/model/edit-shortcut-model.cpp
    ../src/model/shortcut-config.cpp
    ../src/main/global-settings.cpp
    src/model/edit-shortcut-model-test.cpp)
target_include_directories(editShortcutModelTest PRIVATE
    ${UKUI_SHORTCUT_LOCAL_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)
target_link_libraries(editShortcutModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        ukui-quick::platform
        ukui-quick::framework
        ukui-quick::core
        ${EXTERNAL_LIBS}
        ${GIO_LIBRARIES}
        ukui-shortcut
)
add_test(NAME editShortcutModelTest COMMAND $<TARGET_FILE:editShortcutModelTest>)

# shortcutConfigTest
add_executable(shortcutConfigTest
    ../src/model/shortcut-config.cpp
    src/model/shortcut-config-test.cpp)
target_link_libraries(shortcutConfigTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-quick::platform
        ukui-quick::framework
        ukui-quick::core
)
add_test(NAME shortcutConfigTest COMMAND $<TARGET_FILE:shortcutConfigTest>)

# shortcutItemTest
add_executable(shortcutItemTest
    ../src/model/shortcut-item.cpp
    ../src/model/shortcut-config.cpp
    src/model/shortcut-item-test.cpp)
target_include_directories(shortcutItemTest PRIVATE
    ${UKUI_SHORTCUT_LOCAL_INCLUDE_DIRS}
)
target_link_libraries(shortcutItemTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        ukui-quick::platform
        ukui-quick::framework
        ukui-quick::core
        ${EXTERNAL_LIBS}
        ukui-shortcut
)
add_test(NAME shortcutItemTest COMMAND $<TARGET_FILE:shortcutItemTest>)

# shortcutFilterModelTest
add_executable(shortcutFilterModelTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ../src/model/shortcut-model.cpp
    ../src/model/shortcut-item.cpp
    ../src/model/shortcut-filter-model.cpp
    ../src/model/edit-shortcut-model.cpp
    ../src/model/shortcut-config.cpp
    ../src/main/global-settings.cpp
    src/model/shortcut-filter-model-test.cpp)
target_include_directories(shortcutFilterModelTest PRIVATE
    ${UKUI_SHORTCUT_LOCAL_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)
target_link_libraries(shortcutFilterModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        ukui-quick::platform
        ukui-quick::framework
        ukui-quick::core
        ${EXTERNAL_LIBS}
        ${GIO_LIBRARIES}
)
add_test(NAME shortcutFilterModelTest COMMAND $<TARGET_FILE:shortcutFilterModelTest>)

# notificationUtilsTest
add_executable(notificationUtilsTest
    ../src/notification/notification-utils.cpp
    src/notification/notification-utils-test.cpp)
target_link_libraries(notificationUtilsTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME notificationUtilsTest COMMAND $<TARGET_FILE:notificationUtilsTest>)

# dateTimeUtilsTest
add_executable(dateTimeUtilsTest
    ../src/utils/date-time-utils.cpp
    src/utils/date-time-utils-test.cpp
    src/utils/stubs/QGSettings.h
    src/utils/stubs/libkydate-stub.cpp)
target_include_directories(dateTimeUtilsTest BEFORE PRIVATE
    src/utils/stubs
)
target_link_libraries(dateTimeUtilsTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME dateTimeUtilsTest COMMAND $<TARGET_FILE:dateTimeUtilsTest>)

# eventTrackTest
add_executable(eventTrackTest
    ../src/utils/event-track.cpp
    src/utils/event-track-test.cpp
    src/utils/stubs/libkydatacollect-stub.cpp)
target_include_directories(eventTrackTest BEFORE PRIVATE
    src/utils/stubs
)
target_link_libraries(eventTrackTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Qml
)
add_test(NAME eventTrackTest COMMAND $<TARGET_FILE:eventTrackTest>)

# weatherHelperTest
add_executable(weatherHelperTest
    ../src/utils/weather-helper.cpp
    src/utils/stubs/QGSettings
    src/utils/stubs/QGSettings.h
    src/utils/stubs/app-manager.h
    src/utils/stubs/qgsettings-stub.cpp
    src/utils/weather-helper-test.cpp)
target_include_directories(weatherHelperTest BEFORE PRIVATE
    src/utils/stubs
)
target_link_libraries(weatherHelperTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME weatherHelperTest COMMAND $<TARGET_FILE:weatherHelperTest>)

# shortCutManagerTest
set(SHORTCUT_MANAGER_TEST_PLUGIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/short-cut-manager-test-plugins")
file(MAKE_DIRECTORY "${SHORTCUT_MANAGER_TEST_PLUGIN_DIR}")

function(add_shortcut_manager_fake_plugin target source)
    add_library(${target} MODULE
        ukui-shortcut/fake-plugins/${source})
    target_include_directories(${target} PRIVATE
        ../ukui-shortcut
        ../ukui-shortcut/interface
        ukui-shortcut/fake-plugins)
    target_link_libraries(${target}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        ukui-shortcut)
    set_target_properties(${target} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${SHORTCUT_MANAGER_TEST_PLUGIN_DIR}")
endfunction()

add_shortcut_manager_fake_plugin(shortCutManagerValidPlugin fake-shortcut-plugin.cpp)
add_shortcut_manager_fake_plugin(shortCutManagerNullPlugin null-shortcut-plugin.cpp)
add_shortcut_manager_fake_plugin(shortCutManagerWrongTypePlugin wrong-type-plugin.cpp)
add_shortcut_manager_fake_plugin(shortCutManagerWrongVersionPlugin wrong-version-plugin.cpp)
add_shortcut_manager_fake_plugin(shortCutManagerNonShortcutPlugin non-shortcut-plugin.cpp)

add_executable(shortCutManagerTest
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/short-cut-manager-test.cpp)
target_include_directories(shortCutManagerTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface)
target_compile_definitions(shortCutManagerTest PRIVATE
    UKUISHORTCUT_LIBRARY
    PLUGIN_INSTALL_DIRS="${SHORTCUT_MANAGER_TEST_PLUGIN_DIR}"
    SHORTCUT_TRANSLATION_FILE_DIR="${SHORTCUT_MANAGER_TEST_PLUGIN_DIR}/translations")
target_link_libraries(shortCutManagerTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
)
add_dependencies(shortCutManagerTest
    shortCutManagerValidPlugin
    shortCutManagerNullPlugin
    shortCutManagerWrongTypePlugin
    shortCutManagerWrongVersionPlugin
    shortCutManagerNonShortcutPlugin)
add_test(NAME shortCutManagerTest COMMAND $<TARGET_FILE:shortCutManagerTest>)

# statusInfoTest
add_executable(statusInfoTest
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/status-info-test.cpp)
target_include_directories(statusInfoTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface)
target_compile_definitions(statusInfoTest PRIVATE
    UKUISHORTCUT_LIBRARY)
target_link_libraries(statusInfoTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME statusInfoTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:statusInfoTest>)

# wifiDbusQueryTest
add_executable(wifiDbusQueryTest
    ../ukui-shortcut/shortcuts/wifi-shortcut/wifi-dbus-query.h
    ukui-shortcut/wifi-dbus-query-test.cpp)
target_include_directories(wifiDbusQueryTest PRIVATE
    ../ukui-shortcut/shortcuts/wifi-shortcut
)
target_link_libraries(wifiDbusQueryTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME wifiDbusQueryTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:wifiDbusQueryTest>)

# wifiShortcutTest
add_executable(wifiShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/wifi-shortcut/wifi-dbus-query.h
    ../ukui-shortcut/shortcuts/wifi-shortcut/wifi-shortcut.h
    ukui-shortcut/stubs/ukui-quick/platform/dbus-connector.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ../ukui-shortcut/shortcuts/wifi-shortcut/wifi-dbus-query.cpp
    ukui-shortcut/wifi-shortcut-test.cpp)
target_include_directories(wifiShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(wifiShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/wifi-shortcut
)
target_compile_definitions(wifiShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(wifiShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME wifiShortcutTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:wifiShortcutTest>)

# bluetoothShortcutTest
add_executable(bluetoothShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/bluetooth-shortcut/bluetooth-shortcut.h
    ukui-shortcut/stubs/dbus-connector.h
    ukui-shortcut/stubs/ukui-quick/platform/dbus-connector.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/bluetooth-shortcut-test.cpp)
target_include_directories(bluetoothShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(bluetoothShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/bluetooth-shortcut
)
target_compile_definitions(bluetoothShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(bluetoothShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME bluetoothShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:bluetoothShortcutTest>)

# bluetoothConnectorTest
add_executable(bluetoothConnectorTest
    ../ukui-shortcut/shortcuts/bluetooth-shortcut/bluetooth-connector.cpp
    ../ukui-shortcut/shortcuts/bluetooth-shortcut/bluetooth-connector.h
    ukui-shortcut/bluetooth-connector-stubs/QDBusInterface
    ukui-shortcut/bluetooth-connector-stubs/QTime
    ukui-shortcut/shortcuts/bluetooth-connector-test.cpp)
target_include_directories(bluetoothConnectorTest BEFORE PRIVATE
    ukui-shortcut/bluetooth-connector-stubs
)
target_include_directories(bluetoothConnectorTest PRIVATE
    ../ukui-shortcut/shortcuts/bluetooth-shortcut
)
target_link_libraries(bluetoothConnectorTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME bluetoothConnectorTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:bluetoothConnectorTest>)

# volumeShortcutTest
add_executable(volumeShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/volume-shortcut/volume-shortcut.h
    ukui-shortcut/stubs/ukui-quick/platform/dbus-connector.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/volume-shortcut-test.cpp)
target_include_directories(volumeShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(volumeShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/volume-shortcut
)
target_compile_definitions(volumeShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(volumeShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME volumeShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:volumeShortcutTest>)

# brightnessShortcutTest
add_executable(brightnessShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/brightness-shortcut/brightness-shortcut.h
    ukui-shortcut/stubs/dbus-connector.h
    ukui-shortcut/stubs/ukui-quick/platform/dbus-connector.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/brightness-shortcut-test.cpp)
target_include_directories(brightnessShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(brightnessShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/brightness-shortcut
)
target_compile_definitions(brightnessShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(brightnessShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME brightnessShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:brightnessShortcutTest>)

# autoRotationShortcutTest
add_executable(autoRotationShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/auto-rotation-shortcut/auto-rotation-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/auto-rotation-shortcut-test.cpp)
target_include_directories(autoRotationShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/auto-rotation-shortcut
)
target_compile_definitions(autoRotationShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(autoRotationShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME autoRotationShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:autoRotationShortcutTest>)

# statusChangeShortcutTest
add_executable(statusChangeShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/status-change-shortcut/status-change-shortcut.h
    ukui-shortcut/status-change-stubs/kysdk/kysdk-system/libkysysinfo.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/status-change-shortcut-test.cpp)
target_include_directories(statusChangeShortcutTest BEFORE PRIVATE
    ukui-shortcut/status-change-stubs
)
target_include_directories(statusChangeShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/status-change-shortcut
)
target_compile_definitions(statusChangeShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
    TRANSLATION_FILE_DIR="/tmp"
)
target_link_libraries(statusChangeShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME statusChangeShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:statusChangeShortcutTest>)

# clipboardShortcutTest
add_executable(clipboardShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/clipboard-shortcut/clipboard-shortcut.h
    ukui-shortcut/stubs/application-info.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/clipboard-shortcut-test.cpp)
target_include_directories(clipboardShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(clipboardShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/clipboard-shortcut
)
target_compile_definitions(clipboardShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(clipboardShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME clipboardShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:clipboardShortcutTest>)

# clockShortcutTest
add_executable(clockShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/clock-shortcut/clock-shortcut.h
    ukui-shortcut/stubs/application-info.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/clock-shortcut-test.cpp)
target_include_directories(clockShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(clockShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/clock-shortcut
)
target_compile_definitions(clockShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(clockShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME clockShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:clockShortcutTest>)

# connectivityShortcutTest
add_executable(connectivityShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/connectivity-shortcut/connectivity-shortcut.h
    ukui-shortcut/stubs/application-info.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/connectivity-shortcut-test.cpp)
target_include_directories(connectivityShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(connectivityShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/connectivity-shortcut
)
target_compile_definitions(connectivityShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(connectivityShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME connectivityShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:connectivityShortcutTest>)

# darkModeShortcutTest
add_executable(darkModeShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/dark-mode-shortcut/dark-mode-shortcut.h
    ukui-shortcut/stubs/QGSettings
    ukui-shortcut/stubs/QGSettings.h
    ukui-shortcut/stubs/qgsettings-stub.cpp
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/dark-mode-shortcut-test.cpp)
target_include_directories(darkModeShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(darkModeShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/dark-mode-shortcut
)
target_compile_definitions(darkModeShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(darkModeShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME darkModeShortcutTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:darkModeShortcutTest>)

# nightModeShortcutTest
add_executable(nightModeShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/night-mode-shortcut/night-mode-shortcut.h
    ukui-shortcut/stubs/QGSettings
    ukui-shortcut/stubs/QGSettings.h
    ukui-shortcut/stubs/QX11Info
    ukui-shortcut/stubs/qgsettings-stub.cpp
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/night-mode-shortcut-test.cpp)
target_include_directories(nightModeShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(nightModeShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/night-mode-shortcut
)
target_compile_definitions(nightModeShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(nightModeShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME nightModeShortcutTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:nightModeShortcutTest>)

# softFreezeShortcutTest
add_executable(softFreezeShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/soft-freeze-shortcut/soft-freeze-shortcut.h
    ukui-shortcut/stubs/QGSettings
    ukui-shortcut/stubs/QGSettings.h
    ukui-shortcut/stubs/qgsettings-stub.cpp
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/soft-freeze-shortcut-test.cpp)
target_include_directories(softFreezeShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(softFreezeShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/soft-freeze-shortcut
)
target_compile_definitions(softFreezeShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(softFreezeShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME softFreezeShortcutTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:softFreezeShortcutTest>)

# eyeCareModeShortcutTest
add_executable(eyeCareModeShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/eyecare-mode-shortcut/eyecare-mode-shortcut.h
    ukui-shortcut/stubs/QGSettings
    ukui-shortcut/stubs/QGSettings.h
    ukui-shortcut/stubs/dbus-connector.h
    ukui-shortcut/stubs/ukui-quick/platform/dbus-connector.h
    ukui-shortcut/stubs/qgsettings-stub.cpp
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/eyecare-mode-shortcut-test.cpp)
target_include_directories(eyeCareModeShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(eyeCareModeShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/eyecare-mode-shortcut
)
target_compile_definitions(eyeCareModeShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(eyeCareModeShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME eyeCareModeShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:eyeCareModeShortcutTest>)

# flightModeShortcutTest
add_executable(flightModeShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/flight-mode-shortcut/flight-mode-shortcut.h
    ukui-shortcut/stubs/QGSettings
    ukui-shortcut/stubs/QGSettings.h
    ukui-shortcut/stubs/qgsettings-stub.cpp
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/flight-mode-shortcut-test.cpp)
target_include_directories(flightModeShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(flightModeShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/flight-mode-shortcut
)
target_compile_definitions(flightModeShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(flightModeShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME flightModeShortcutTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:flightModeShortcutTest>)

# multiScreenShortcutTest
add_executable(multiScreenShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/multi-screen-shortcut/multi-screen-shortcut.h
    ukui-shortcut/stubs/QProcess
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/multi-screen-shortcut-test.cpp)
target_include_directories(multiScreenShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(multiScreenShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/multi-screen-shortcut
)
target_compile_definitions(multiScreenShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(multiScreenShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME multiScreenShortcutTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:multiScreenShortcutTest>)

# notebookShortcutTest
add_executable(notebookShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/notebook-shortcut/notebook-shortcut.h
    ukui-shortcut/stubs/application-info.h
    ukui-shortcut/stubs/QProcess
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/notebook-shortcut-test.cpp)
target_include_directories(notebookShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(notebookShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/notebook-shortcut
)
target_compile_definitions(notebookShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(notebookShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME notebookShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:notebookShortcutTest>)

# powerModeShortcutTest
add_executable(powerModeShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/power-mode-shortcut/power-mode-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/power-mode-shortcut-test.cpp)
target_include_directories(powerModeShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/power-mode-shortcut
)
target_compile_definitions(powerModeShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(powerModeShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME powerModeShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:powerModeShortcutTest>)

# screenshotShortcutTest
add_executable(screenshotShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/screenshot-shortcut/screenshot-shortcut.h
    ukui-shortcut/stubs/application-info.h
    ukui-shortcut/stubs/QProcess
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/screenshot-shortcut-test.cpp)
target_include_directories(screenshotShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(screenshotShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/screenshot-shortcut
)
target_compile_definitions(screenshotShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(screenshotShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME screenshotShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:screenshotShortcutTest>)

# servicesAndSupportsShortcutTest
add_executable(servicesAndSupportsShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/services-and-supports-shortcut/services-and-supports-shortcut.h
    ukui-shortcut/stubs/application-info.h
    ukui-shortcut/stubs/QProcess
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/services-and-supports-shortcut-test.cpp)
target_include_directories(servicesAndSupportsShortcutTest BEFORE PRIVATE
    ukui-shortcut/stubs
)
target_include_directories(servicesAndSupportsShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/services-and-supports-shortcut
)
target_compile_definitions(servicesAndSupportsShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(servicesAndSupportsShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME servicesAndSupportsShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:servicesAndSupportsShortcutTest>)

# systemSettingShortcutTest
add_executable(systemSettingShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/system-setting-shortcut/system-setting-shortcut.h
    ukui-shortcut/system-setting-stubs/QDBusInterface
    ukui-shortcut/system-setting-stubs/QDBusReply
    ukui-shortcut/stubs/QProcess
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/system-setting-shortcut-test.cpp)
target_include_directories(systemSettingShortcutTest BEFORE PRIVATE
    ukui-shortcut/system-setting-stubs
)
target_include_directories(systemSettingShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/system-setting-shortcut
)
target_compile_definitions(systemSettingShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(systemSettingShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME systemSettingShortcutTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:systemSettingShortcutTest>)

# vpnShortcutTest
add_executable(vpnShortcutTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/status-info.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/shortcuts/vpn-shortcut/vpn-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ukui-shortcut/shortcuts/vpn-shortcut-test.cpp)
target_include_directories(vpnShortcutTest PRIVATE
    ../ukui-shortcut
    ../ukui-shortcut/interface
    ../ukui-shortcut/shortcuts/vpn-shortcut
)
target_compile_definitions(vpnShortcutTest PRIVATE
    UKUISHORTCUT_LIBRARY
)
target_link_libraries(vpnShortcutTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME vpnShortcutTest
    COMMAND dbus-run-session ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:vpnShortcutTest>)

# logUtilsTest
add_executable(logUtilsTest
    ../src/log-utils.cpp
    src/log-utils-test.cpp)
target_link_libraries(logUtilsTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME logUtilsTest COMMAND $<TARGET_FILE:logUtilsTest>)

# sidebarDbusServiceTest
add_executable(sidebarDbusServiceTest
    ../src/sidebar-dbus-service.h
    src/sidebar-dbus-service-wrapper.cpp
    src/sidebar-dbus-service-test.cpp
    src/stubs/sidebar-main.h
    src/stubs/sidebaradaptor.h
    src/stubs/main-stubs.cpp)
target_include_directories(sidebarDbusServiceTest BEFORE PRIVATE
    src/stubs
)
target_link_libraries(sidebarDbusServiceTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME sidebarDbusServiceTest COMMAND $<TARGET_FILE:sidebarDbusServiceTest>)

# mainTest
add_executable(mainTest
    src/main-entry-wrapper.cpp
    src/main-test.cpp
    src/stubs/qtsingleapplication.h
    src/stubs/sidebar-main.h
    src/stubs/log-utils.h
    src/stubs/qtsingleapplication-stubs.cpp
    src/stubs/main-stubs.cpp)
target_include_directories(mainTest BEFORE PRIVATE
    src/stubs
)
target_link_libraries(mainTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
)
target_compile_definitions(mainTest PRIVATE
    VERSION="test-version"
    TRANSLATION_FILE_DIR="/tmp"
)
add_test(NAME mainTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:mainTest>)

# sidebarMainTest
add_executable(sidebarMainTest
    ../src/sidebar-main.h
    src/sidebar-main-wrapper.cpp
    src/sidebar-main-test.cpp
    src/stubs/sidebar-main-deps.h
    src/windows/stubs/date-time-utils.h
    src/windows/stubs/notification-window.h
    src/windows/stubs/short-cut-manager.h
    src/windows/stubs/value-type.h
    src/windows/stubs/shortcut-model.h
    src/windows/stubs/shared-engine-view.h
    src/windows/stubs/window-helper.h
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/sidebar-window-helper.h
    src/windows/stubs/notification-model.h
    src/windows/stubs/notification-group-model.h
    src/windows/stubs/popup-notification-model.h
    src/windows/stubs/urgency-notification-model.h
    src/windows/stubs/event-track.h
    src/windows/stubs/QX11Info
    src/stubs/sidebar-main-notification-center-stubs.cpp
    src/windows/stubs/notification-window-stubs.cpp)
target_include_directories(sidebarMainTest BEFORE PRIVATE
    src/stubs
    src/windows/stubs
)
target_link_libraries(sidebarMainTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME sidebarMainTest COMMAND $<TARGET_FILE:sidebarMainTest>)

# notificationWindowTest
add_executable(notificationWindowTest
    ../src/windows/notification-window.cpp
    src/windows/notification-window-test.cpp
    src/windows/stubs/shared-engine-view.h
    src/windows/stubs/window-helper.h
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/event-track.h
    src/windows/stubs/sidebar-window-helper.h
    src/windows/stubs/notification-model.h
    src/windows/stubs/notification-group-model.h
    src/windows/stubs/popup-notification-model.h
    src/windows/stubs/urgency-notification-model.h
    src/windows/stubs/shortcut-model.h
    src/windows/stubs/notification-window-stubs.cpp)
target_include_directories(notificationWindowTest BEFORE PRIVATE
    src/windows/stubs
)
target_link_libraries(notificationWindowTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME notificationWindowTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:notificationWindowTest>)

# popupNotificationWindowTest
add_executable(popupNotificationWindowTest
    ../src/windows/popup-notification-window.cpp
    src/windows/popup-notification-window-test.cpp
    src/windows/stubs/shared-engine-view.h
    src/windows/stubs/window-helper.h
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/event-track.h
    src/windows/stubs/sidebar-window-helper.h
    src/windows/stubs/notification-model.h
    src/windows/stubs/notification-group-model.h
    src/windows/stubs/popup-notification-model.h
    src/windows/stubs/urgency-notification-model.h
    src/windows/stubs/shortcut-model.h
    src/windows/stubs/notification-window-stubs.cpp)
target_include_directories(popupNotificationWindowTest BEFORE PRIVATE
    src/windows/stubs
)
target_link_libraries(popupNotificationWindowTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME popupNotificationWindowTest COMMAND $<TARGET_FILE:popupNotificationWindowTest>)

# urgencyNotificationWindowTest
add_executable(urgencyNotificationWindowTest
    ../src/windows/urgency-notification-window.cpp
    src/windows/urgency-notification-window-test.cpp
    src/windows/stubs/shared-engine-view.h
    src/windows/stubs/window-helper.h
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/event-track.h
    src/windows/stubs/sidebar-window-helper.h
    src/windows/stubs/settings.h
    src/windows/stubs/short-cut-manager.h
    src/windows/stubs/shortcut-model.h
    src/windows/stubs/shortcut-filter-model.h
    src/windows/stubs/value-type.h
    src/windows/stubs/notification-model.h
    src/windows/stubs/notification-group-model.h
    src/windows/stubs/popup-notification-model.h
    src/windows/stubs/urgency-notification-model.h
    src/windows/stubs/QGSettings
    src/windows/stubs/QX11Info
    src/windows/stubs/KWindowSystem
    src/windows/stubs/KWindowEffects
    src/windows/stubs/kysdk/applications/windowmanager/windowmanager.h
    src/windows/stubs/notification-window-stubs.cpp)
target_include_directories(urgencyNotificationWindowTest BEFORE PRIVATE
    src/windows/stubs
)
target_include_directories(urgencyNotificationWindowTest PRIVATE
    ../src/notification
    ../src/windows
)
target_link_libraries(urgencyNotificationWindowTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
target_compile_options(urgencyNotificationWindowTest PRIVATE -include QGSettings)
add_test(NAME urgencyNotificationWindowTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QSG_RHI_BACKEND=software LIBGL_ALWAYS_SOFTWARE=1 $<TARGET_FILE:urgencyNotificationWindowTest>)

# rightHandGestureViewTest
add_executable(rightHandGestureViewTest
    ../src/windows/right-hand-gesture-view.cpp
    src/windows/right-hand-gesture-view-test.cpp
    src/windows/stubs/shared-engine-view.h
    src/windows/stubs/window-helper.h
    src/windows/stubs/settings.h
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/event-track.h
    src/windows/stubs/sidebar-window-helper.h
    src/windows/stubs/notification-model.h
    src/windows/stubs/notification-group-model.h
    src/windows/stubs/popup-notification-model.h
    src/windows/stubs/urgency-notification-model.h
    src/windows/stubs/shortcut-model.h
    src/windows/stubs/notification-window-stubs.cpp)
target_include_directories(rightHandGestureViewTest BEFORE PRIVATE
    src/windows/stubs
)
target_link_libraries(rightHandGestureViewTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME rightHandGestureViewTest COMMAND $<TARGET_FILE:rightHandGestureViewTest>)

# shortcutsWindowTest
add_executable(shortcutsWindowTest
    ../src/windows/shortcuts-window.cpp
    src/windows/shortcuts-window-test.cpp
    src/windows/stubs/shared-engine-view.h
    src/windows/stubs/window-helper.h
    src/windows/stubs/settings.h
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/event-track.h
    src/windows/stubs/sidebar-window-helper.h
    src/windows/stubs/notification-model.h
    src/windows/stubs/notification-group-model.h
    src/windows/stubs/popup-notification-model.h
    src/windows/stubs/urgency-notification-model.h
    src/windows/stubs/short-cut-manager.h
    src/windows/stubs/shortcut-model.h
    src/windows/stubs/shortcut-filter-model.h
    src/windows/stubs/notification-window-stubs.cpp)
target_include_directories(shortcutsWindowTest BEFORE PRIVATE
    src/windows/stubs
)
target_link_libraries(shortcutsWindowTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME shortcutsWindowTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:shortcutsWindowTest>)

# sidebarViewTest
add_executable(sidebarViewTest
    ../src/windows/sidebar-view.cpp
    ../src/windows/window-blur-helper.h
    src/windows/sidebar-view-test.cpp
    src/windows/stubs/shared-engine-view.h
    src/windows/stubs/window-helper.h
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/sidebar-window-helper.h
    src/windows/stubs/notification-model.h
    src/windows/stubs/notification-group-model.h
    src/windows/stubs/notification-window.h
    src/windows/stubs/event-track.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/popup-notification-model.h
    src/windows/stubs/urgency-notification-model.h
    src/windows/stubs/shortcut-model.h
    src/windows/stubs/window-blur-helper-stubs.cpp
    src/windows/stubs/notification-center-window-stubs.cpp
    src/windows/stubs/notification-window-stubs.cpp)
target_include_directories(sidebarViewTest BEFORE PRIVATE
    src/windows/stubs
)
target_link_libraries(sidebarViewTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME sidebarViewTest COMMAND $<TARGET_FILE:sidebarViewTest>)

# sidebarWindowHelperTest
add_executable(sidebarWindowHelperTest
    ../src/windows/sidebar-window-helper.cpp
    ../src/windows/window-type.h
    src/windows/sidebar-window-helper-test.cpp
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/sidebar-window-helper-deps-stubs.cpp)
target_include_directories(sidebarWindowHelperTest BEFORE PRIVATE
    src/windows/stubs
)
target_link_libraries(sidebarWindowHelperTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME sidebarWindowHelperTest COMMAND $<TARGET_FILE:sidebarWindowHelperTest>)

# statusBarViewTest
add_executable(statusBarViewTest
    ../src/windows/status-bar-view.cpp
    src/windows/status-bar-view-test.cpp
    src/windows/stubs/shared-engine-view.h
    src/windows/stubs/window-helper.h
    src/windows/stubs/settings.h
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/event-track.h
    src/windows/stubs/sidebar-window-helper.h
    src/windows/stubs/notification-model.h
    src/windows/stubs/notification-group-model.h
    src/windows/stubs/popup-notification-model.h
    src/windows/stubs/urgency-notification-model.h
    src/windows/stubs/shortcut-model.h
    src/windows/stubs/notification-window-stubs.cpp)
target_include_directories(statusBarViewTest BEFORE PRIVATE
    src/windows/stubs
)
target_link_libraries(statusBarViewTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME statusBarViewTest COMMAND $<TARGET_FILE:statusBarViewTest>)

# tabletPopupViewTest
add_executable(tabletPopupViewTest
    ../src/windows/tablet-popup-view.cpp
    ../src/windows/tablet-popup-view.h
    src/windows/tablet-popup-view-test.cpp
    src/windows/stubs/shared-engine-view.h
    src/windows/stubs/window-helper.h
    src/windows/stubs/settings.h
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/event-track.h
    src/windows/stubs/sidebar-window-helper.h
    src/windows/stubs/notification-model.h
    src/windows/stubs/notification-group-model.h
    src/windows/stubs/popup-notification-model.h
    src/windows/stubs/urgency-notification-model.h
    src/windows/stubs/group-cache-proxy-model.h
    src/windows/stubs/date-time-utils.h
    src/windows/stubs/color-helper.h
    src/windows/stubs/value-type.h
    src/windows/stubs/shortcut-model.h
    src/windows/stubs/notification-window-stubs.cpp)
target_include_directories(tabletPopupViewTest BEFORE PRIVATE
    src/windows/stubs
)
target_link_libraries(tabletPopupViewTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME tabletPopupViewTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software $<TARGET_FILE:tabletPopupViewTest>)

# windowBlurHelperTest
add_executable(windowBlurHelperTest
    ../src/windows/window-blur-helper.cpp
    ../src/windows/window-blur-helper.h
    src/windows/window-blur-helper-test.cpp
    src/windows/stubs/window-helper.h
    src/windows/stubs/QX11Info
    src/windows/stubs/shared-engine-view.h
    src/windows/stubs/global-settings.h
    src/windows/stubs/screen-monitor.h
    src/windows/stubs/hand-gesture-helper.h
    src/windows/stubs/event-track.h
    src/windows/stubs/sidebar-window-helper.h
    src/windows/stubs/notification-model.h
    src/windows/stubs/notification-group-model.h
    src/windows/stubs/popup-notification-model.h
    src/windows/stubs/urgency-notification-model.h
    src/windows/stubs/xcb/xcb.h
    src/windows/stubs/xcb-stubs.cpp
    src/windows/stubs/shortcut-model.h
    src/windows/stubs/notification-window-stubs.cpp)
target_include_directories(windowBlurHelperTest BEFORE PRIVATE
    src/windows/stubs
)
target_link_libraries(windowBlurHelperTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Test
        xcb
)
add_test(NAME windowBlurHelperTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:windowBlurHelperTest>)

# urgencyNotificationModelTest
pkg_check_modules(UKUI_NOTIFICATION REQUIRED ukui-notification)
add_executable(urgencyNotificationModelTest
    ../src/notification/notification-utils.cpp
    ../src/notification/urgency-notification-model.cpp
    src/notification/urgency-notification-model-test.cpp)
target_include_directories(urgencyNotificationModelTest PRIVATE
    ${UKUI_NOTIFICATION_INCLUDE_DIRS}
)
target_link_libraries(urgencyNotificationModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ${UKUI_NOTIFICATION_LIBRARIES}
)
add_test(NAME urgencyNotificationModelTest COMMAND $<TARGET_FILE:urgencyNotificationModelTest>)

# groupCacheProxyModelTest
add_executable(groupCacheProxyModelTest
    ../src/notification/group-cache-proxy-model.cpp
    src/notification/group-cache-proxy-model-test.cpp)
target_include_directories(groupCacheProxyModelTest PRIVATE
    ${UKUI_NOTIFICATION_INCLUDE_DIRS}
)
target_link_libraries(groupCacheProxyModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ${UKUI_NOTIFICATION_LIBRARIES}
)
add_test(NAME groupCacheProxyModelTest COMMAND $<TARGET_FILE:groupCacheProxyModelTest>)

# notificationGroupModelTest
add_executable(notificationGroupModelTest
    ../src/notification/notification-group-model.cpp
    src/notification/notification-group-model-test.cpp)
target_include_directories(notificationGroupModelTest PRIVATE
    ${UKUI_NOTIFICATION_INCLUDE_DIRS}
)
target_link_libraries(notificationGroupModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ${UKUI_NOTIFICATION_LIBRARIES}
)
add_test(NAME notificationGroupModelTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:notificationGroupModelTest>)

# notificationManagerTest
add_executable(notificationManagerTest
    ../src/notification/notification-manager.cpp
    src/notification/notification-manager-test.cpp
    src/notification/stubs/app-manager.h
    src/notification/stubs/notification-helper.h
    src/notification/stubs/notification-manager-stubs.cpp)
target_include_directories(notificationManagerTest BEFORE PRIVATE
    src/notification/stubs
)
target_link_libraries(notificationManagerTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME notificationManagerTest COMMAND $<TARGET_FILE:notificationManagerTest>)

# popupNotificationModelTest
add_executable(popupNotificationModelTest
    ../src/notification/popup-notification-model.cpp
    src/notification/popup-notification-model-test.cpp
    src/notification/stubs/global-settings.h
    src/notification/stubs/screen-monitor.h
    src/notification/stubs/notification-ui-stubs.cpp)
target_include_directories(popupNotificationModelTest BEFORE PRIVATE
    src/notification/stubs
    ${UKUI_NOTIFICATION_INCLUDE_DIRS}
)
target_link_libraries(popupNotificationModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ${UKUI_NOTIFICATION_LIBRARIES}
)
add_test(NAME popupNotificationModelTest COMMAND $<TARGET_FILE:popupNotificationModelTest>)

# notificationModelTest
add_executable(notificationModelTest
    ../src/notification/notification-utils.cpp
    ../src/notification/notification-model.cpp
    src/notification/notification-model-test.cpp
    src/notification/stubs/popup-notification.h
    src/notification/stubs/notification-close-reason.h
    src/notification/stubs/notification-client.h
    src/notification/stubs/applications-settings.h
    src/notification/stubs/notification-global-settings.h
    src/notification/stubs/global-settings.h
    src/notification/stubs/screen-monitor.h
    src/notification/stubs/notification-ui-stubs.cpp
    src/notification/stubs/notification-model-stubs.cpp)
target_include_directories(notificationModelTest BEFORE PRIVATE
    src/notification/stubs
)
target_link_libraries(notificationModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
)
add_test(NAME notificationModelTest
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen $<TARGET_FILE:notificationModelTest>)

# shortcutModelIsolatedTest
add_executable(shortcutModelIsolatedTest
    ../ukui-shortcut/interface/plugin-common-data.h
    ../ukui-shortcut/interface/value-type.h
    ../ukui-shortcut/interface/ukui-shortcut.h
    ../ukui-shortcut/interface/ukui-shortcut-plugin.cpp
    ../ukui-shortcut/interface/status-info.cpp
    ../src/model/plugin-state.h
    ../src/model/shortcut-model.cpp
    ../src/model/shortcut-item.cpp
    ../src/model/shortcut-filter-model.cpp
    ../src/model/edit-shortcut-model.cpp
    ../src/model/shortcut-config.cpp
    ../src/main/global-settings.cpp
    src/model/shortcut-model-isolated-test.cpp)
target_include_directories(shortcutModelIsolatedTest PRIVATE
    ${UKUI_SHORTCUT_LOCAL_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
)
target_link_libraries(shortcutModelIsolatedTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::DBus
        ukui-quick::platform
        ukui-quick::framework
        ukui-quick::core
        ${EXTERNAL_LIBS}
        ${GIO_LIBRARIES}
)
add_test(NAME shortcutModelIsolatedTest COMMAND $<TARGET_FILE:shortcutModelIsolatedTest>)
