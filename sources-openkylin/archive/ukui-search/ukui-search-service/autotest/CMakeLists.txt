set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Test REQUIRED)

include_directories(..)

add_executable(cgroupHelperTest
        cgroup-helper-test.cpp
        ../cgroup-helper.cpp)
target_link_libraries(cgroupHelperTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Test)
add_test(cgroupHelperTest ${CMAKE_BINARY_DIR}/ukui-search-service/autotest/cgroupHelperTest)
