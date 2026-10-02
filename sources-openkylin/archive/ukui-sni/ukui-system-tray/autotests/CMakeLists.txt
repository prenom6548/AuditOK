set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Test DBus Qml Widgets Quick REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Test DBus Qml Widgets Quick REQUIRED)
find_package(ukui-quick COMPONENTS framework)

include_directories(../)
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/imitates)

add_executable(trayItemsModelTest
    ../plugin/tray-items-model.cpp
    ../plugin/tray-item.cpp
    ../plugin/item-group-model.cpp
    ../plugin/items-filter-model.cpp
    ../plugin/control-panel-model.cpp
    ../plugin/security-function-control.cpp
    trayItemsModelTest.cpp)

include_directories(${CMAKE_SOURCE_DIR}/status-notifier-host)
include_directories(${CMAKE_SOURCE_DIR}/common)

target_link_libraries(trayItemsModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ukui-sni-host
        ukui-quick::framework
        ukui-quick::platform
)
add_test(NAME trayItemsModelTest COMMAND ${CMAKE_CURRENT_BINARY_DIR}/trayItemsModelTest)

add_executable(securityFunctionControlTest
    imitates/function-control/function-control.cpp
    ../plugin/security-function-control.cpp
    securityFunctionControlTest.cpp)
target_include_directories(securityFunctionControlTest
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/imitates/function-control
)

target_link_libraries(securityFunctionControlTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
)
add_test(NAME securityFunctionControlTest COMMAND ${CMAKE_CURRENT_BINARY_DIR}/securityFunctionControlTest)

add_executable(controlPanelModelTest
    ../plugin/control-panel-model.cpp
    ../plugin/items-filter-model.cpp
    ../plugin/item-group-model.cpp
    ../plugin/tray-items-model.cpp
    ../plugin/tray-item.cpp
    ../plugin/security-function-control.cpp
    controlPanelModelTest.cpp)
target_link_libraries(controlPanelModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ukui-sni-host
        ukui-quick::framework
        ukui-quick::platform
)
add_test(NAME controlPanelModelTest COMMAND ${CMAKE_CURRENT_BINARY_DIR}/controlPanelModelTest)

add_executable(itemGroupModelTest
    ../plugin/item-group-model.cpp
    ../plugin/items-filter-model.cpp
    ../plugin/tray-items-model.cpp
    ../plugin/tray-item.cpp
    ../plugin/control-panel-model.cpp
    ../plugin/security-function-control.cpp
    itemGroupModelTest.cpp)
target_link_libraries(itemGroupModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ukui-sni-host
        ukui-quick::framework
        ukui-quick::platform
)
add_test(NAME itemGroupModelTest COMMAND ${CMAKE_CURRENT_BINARY_DIR}/itemGroupModelTest)

add_executable(itemsFilterModelTest
    ../plugin/items-filter-model.cpp
    ../plugin/item-group-model.cpp
    ../plugin/tray-items-model.cpp
    ../plugin/tray-item.cpp
    ../plugin/control-panel-model.cpp
    ../plugin/security-function-control.cpp
    itemsFilterModelTest.cpp)
target_link_libraries(itemsFilterModelTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ukui-sni-host
        ukui-quick::framework
        ukui-quick::platform
)
add_test(NAME itemsFilterModelTest COMMAND ${CMAKE_CURRENT_BINARY_DIR}/itemsFilterModelTest)

add_executable(trayItemTest
    ../plugin/tray-item.cpp
    trayItemTest.cpp)
target_link_libraries(trayItemTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ukui-sni-host
        Qt5Xdg
        Qt5XdgIconLoader
)
add_test(NAME trayItemTest COMMAND ${CMAKE_CURRENT_BINARY_DIR}/trayItemTest)

add_executable(systemTrayPluginTest
    ../plugin/system-tray-plugin.cpp
    ../plugin/tray-items-model.cpp
    ../plugin/tray-item.cpp
    ../plugin/item-group-model.cpp
    ../plugin/items-filter-model.cpp
    ../plugin/control-panel-model.cpp
    ../plugin/security-function-control.cpp
    systemTrayPluginTest.cpp)

target_link_libraries(systemTrayPluginTest
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ukui-sni-host
        ukui-quick::framework
        ukui-quick::platform
        ukui-search
        Qt5Xdg
        Qt5XdgIconLoader
)
add_test(NAME systemTrayPluginTest COMMAND ${CMAKE_CURRENT_BINARY_DIR}/systemTrayPluginTest)
