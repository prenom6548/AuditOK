cmake_minimum_required(VERSION 3.16)
project(ukui-quick)
set(ROOT_PROJECT_NAME ukui-quick)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(UKUI_QUICK_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

set(UKUI_QUICK_CMAKE_EXTEND_MODULE_DIR
        "${CMAKE_SOURCE_DIR}/cmake-extend/modules")

list(PREPEND CMAKE_MODULE_PATH "${UKUI_QUICK_CMAKE_EXTEND_MODULE_DIR}")

option(BUILD_TEST "Build automated tests" OFF)
option(BUILD_INTERACTIVE_TESTS "Build interactive/manual test apps when BUILD_TEST is ON" ON)
option(UKUI_QUICK_ENABLE_COVERAGE
       "Instrument BUILD_TEST targets with GCC coverage flags"
       ON)

if(BUILD_TEST)
    enable_testing()
    if(UKUI_QUICK_ENABLE_COVERAGE)
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
    endif()
endif()

include(CMakePackageConfigHelpers)
add_subdirectory(core)
add_subdirectory(platform/ukui/lottie)
add_subdirectory(items)
add_subdirectory(platform)
add_subdirectory(framework)
add_subdirectory(modules)
add_subdirectory(cmake-extend)

set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/ukui-quick")
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/ukui-quick-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-config.cmake
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR}
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-config-version.cmake
        VERSION ${UKUI_QUICK_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}
        COMPONENT Devel
)
