cmake_minimum_required(VERSION 3.14)
project(cmake-extend)

set(MODULES_CMAKE_EXTEND_INSTALL_DIR "/usr/share/cmake/ukui-quick-cmake-extend/modules/")
set(CONFIG_CMAKE_EXTEND_INSTALL_DIR "/usr/share/cmake/ukui-quick-cmake-extend/cmake/")
set(STYLE_CMAKE_EXTEND_INSTALL_DIR "/usr/share/cmake/ukui-quick-cmake-extend/style/")


file(GLOB installCMakeExtendFiles ${CMAKE_CURRENT_SOURCE_DIR}/modules/*)
install(FILES ${installCMakeExtendFiles} DESTINATION ${MODULES_CMAKE_EXTEND_INSTALL_DIR})

file(GLOB installCMakeExtendStyleFiles ${CMAKE_CURRENT_SOURCE_DIR}/style/*.css)
install(FILES ${installCMakeExtendStyleFiles} DESTINATION ${STYLE_CMAKE_EXTEND_INSTALL_DIR})

include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/ukui-quick-cmake-extend-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-cmake-extend-config.cmake"
        INSTALL_DESTINATION  ${CONFIG_CMAKE_EXTEND_INSTALL_DIR}
        PATH_VARS MODULES_CMAKE_EXTEND_INSTALL_DIR STYLE_CMAKE_EXTEND_INSTALL_DIR
)


write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-cmake-extend-config-version.cmake
        VERSION ${UKUI_QUICK_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-cmake-extend-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-quick-cmake-extend-config-version.cmake
        DESTINATION ${CONFIG_CMAKE_EXTEND_INSTALL_DIR})

