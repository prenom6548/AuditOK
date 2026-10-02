message(STATUS "Building ukui-voice-wing")

add_subdirectory(log)
add_subdirectory(config)
add_subdirectory(recognizer)
add_subdirectory(window)
add_subdirectory(core)

if(BUILD_FCITX4)
    add_subdirectory(fcitx4)
endif()
if(BUILD_FCITX5)
    add_subdirectory(fcitx5)
endif()
