set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Place non-interactive framework automated tests here.
# Keep interactive test applications under `framework/test`.

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Qml Quick Test Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Qml Quick Test Widgets REQUIRED)

function(add_framework_autotest target_name source_file)
    add_executable(${target_name}
        ${source_file}
    )

    target_include_directories(${target_name} PRIVATE
        ..
        ../config
        ../widget
        ../widget-ui
        ../../core
    )

    target_link_libraries(${target_name} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        ukui-quick::core
        ukui-quick::framework
    )

    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

add_framework_autotest(test-config test-config.cpp)
add_framework_autotest(test-ini-config test-ini-config.cpp)
add_framework_autotest(test-config-loader test-config-loader.cpp)
add_framework_autotest(test-config-property-map test-config-property-map.cpp)
add_framework_autotest(test-widget-metadata test-widget-metadata.cpp)
add_framework_autotest(test-widget-content test-widget-content.cpp)
add_framework_autotest(test-widget-loader test-widget-loader.cpp)
add_framework_autotest(test-widget test-widget.cpp)
add_framework_autotest(test-widget-container test-widget-container.cpp)
add_framework_autotest(test-island test-island.cpp)
add_framework_autotest(test-island-view test-island-view.cpp)
add_framework_autotest(test-widget-bridge test-widget-bridge.cpp)
add_framework_autotest(test-widget-item test-widget-item.cpp)
add_framework_autotest(test-widget-container-item test-widget-container-item.cpp)
