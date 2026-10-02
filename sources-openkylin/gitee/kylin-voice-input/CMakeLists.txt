cmake_minimum_required(VERSION 3.10)

project(ukui-voice-wing)

include(GNUInstallDirs)

option(BUILD_FCITX5 "Build fcitx5 module" ON)
option(BUILD_FCITX4 "Build fcitx4 module" OFF)
option(BUILD_TESTS "Build tests" OFF)

add_subdirectory(src)
add_subdirectory(translations)
add_subdirectory(data)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()