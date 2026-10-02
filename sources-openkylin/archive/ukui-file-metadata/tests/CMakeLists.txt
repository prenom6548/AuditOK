remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_executable(dump dump.cpp)

target_include_directories(dump  PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/src")

target_link_libraries(dump
    Qt${QT_VERSION_MAJOR}::Core
    ukui-file-metadata
)

configure_file(package-config-test.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/package-config-test.cmake
        @ONLY)
add_test(PackageConfigTest
        ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/package-config-test.cmake)
