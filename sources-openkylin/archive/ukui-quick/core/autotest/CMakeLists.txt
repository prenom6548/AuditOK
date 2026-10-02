set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Place non-interactive core automated tests here.
# Name executables with `test-*` or `*_test` so `tests/runUnitTest.sh` can discover them.

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Qml Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Qml Test REQUIRED)

add_executable(test-margin
    test-margin.cpp
    ../margin.cpp
    ../margin.h
)

target_include_directories(test-margin PRIVATE ..)

target_link_libraries(test-margin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME test-margin COMMAND test-margin)

add_executable(test-shared-engine-component
    test-shared-engine-component.cpp
)

target_include_directories(test-shared-engine-component PRIVATE ..)

target_link_libraries(test-shared-engine-component PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::core
)

add_test(NAME test-shared-engine-component COMMAND test-shared-engine-component)

add_executable(test-shared-engine-view
    test-shared-engine-view.cpp
)

target_include_directories(test-shared-engine-view PRIVATE ..)

target_link_libraries(test-shared-engine-view PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Test
    ukui-quick::core
)

add_test(NAME test-shared-engine-view COMMAND test-shared-engine-view)

add_executable(test-shared-qml-engine
    test-shared-qml-engine.cpp
    ../shared-qml-engine.cpp
    ../shared-qml-engine.h
)

target_include_directories(test-shared-qml-engine PRIVATE ..)

target_link_libraries(test-shared-qml-engine PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME test-shared-qml-engine COMMAND test-shared-qml-engine)
