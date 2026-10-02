set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include_directories(../)

add_executable(settingsPropertiesInfoTest
        settings-properties-info-test.cpp
        ../notification-settings/settings-properties-info.cpp)

target_link_libraries(settingsPropertiesInfoTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
        ukui-notification
        )
add_test(settingsPropertiesInfoTest ${CMAKE_BINARY_DIR}/autotests/settingsPropertiesInfoTest)
