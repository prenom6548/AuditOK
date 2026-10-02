set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Place non-interactive window-thumbnail automated tests here.
# Keep interactive preview demos under `modules/window-thumbnail/core/test`.

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Qml Quick Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Qml Quick Test REQUIRED)

add_executable(test-window-thumbnail-config
    test-window-thumbnail-config.cpp
    ../window-thumbnail-config.cpp
    ../window-thumbnail-config.h
)

target_include_directories(test-window-thumbnail-config PRIVATE ..)

target_link_libraries(test-window-thumbnail-config PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME test-window-thumbnail-config COMMAND test-window-thumbnail-config)

add_executable(test-window-thumbnail-plugin
    test-window-thumbnail-plugin.cpp
)

target_include_directories(test-window-thumbnail-plugin PRIVATE
    ..
)

target_link_libraries(test-window-thumbnail-plugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Test
    ukui-window-thumbnail
)

add_test(NAME test-window-thumbnail-plugin COMMAND test-window-thumbnail-plugin)

add_executable(test-player-items-model
    test-player-items-model.cpp
)

target_include_directories(test-player-items-model PRIVATE
    ..
)

target_link_libraries(test-player-items-model PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Test
    ukui-window-thumbnail
)

add_test(NAME test-player-items-model COMMAND test-player-items-model)

add_executable(test-window-thumbnail-mpris-model
    test-window-thumbnail-mpris-model.cpp
)

target_include_directories(test-window-thumbnail-mpris-model PRIVATE
    ..
)

target_link_libraries(test-window-thumbnail-mpris-model PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Test
    ukui-window-thumbnail
)

add_test(NAME test-window-thumbnail-mpris-model COMMAND test-window-thumbnail-mpris-model)

add_executable(test-thumbnail-item
    test-thumbnail-item.cpp
    ../core/thumbnail-item.cpp
    ../core/thumbnail-item.h
    ../core/scene-graph-resource-release-queue.cpp
    ../core/scene-graph-resource-release-queue.h
    ../core/thumbnail.h
    ../core/thumbnail-buffer.h
)

target_include_directories(test-thumbnail-item PRIVATE
    ..
    ../core
)

target_link_libraries(test-thumbnail-item PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME test-thumbnail-item COMMAND test-thumbnail-item)

add_executable(test-scene-graph-resource-release-queue
    test-scene-graph-resource-release-queue.cpp
    ../core/scene-graph-resource-release-queue.cpp
    ../core/scene-graph-resource-release-queue.h
    ../core/thumbnail-buffer.h
)

target_include_directories(test-scene-graph-resource-release-queue PRIVATE
    ..
    ../core
)

target_link_libraries(test-scene-graph-resource-release-queue PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME test-scene-graph-resource-release-queue COMMAND test-scene-graph-resource-release-queue)
