message(STATUS "Building ukui-voice-wing core")

find_package(Qt5 COMPONENTS Core Gui DBus REQUIRED)
find_package(KF5GlobalAccel REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(PulseAudio REQUIRED libpulse)

add_library(${PROJECT_NAME}-core STATIC
    shortcut.cpp
    shortcut.h
    voiceInput.cpp
    voiceInput.h
    audioInput.cpp
    audioInput.h
)

set_target_properties(${PROJECT_NAME}-core PROPERTIES AUTOMOC ON)

target_include_directories(${PROJECT_NAME}-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../recognizer>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../window>
    PRIVATE
        ${PulseAudio_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}-core
    PRIVATE
        ${PROJECT_NAME}-config
        ${PROJECT_NAME}-recognizer
        ${PROJECT_NAME}-log
        -Wl,--whole-archive
        ${PROJECT_NAME}-window
        -Wl,--no-whole-archive
        KF5::GlobalAccel
        Qt5::DBus
        ${PulseAudio_LIBRARIES}
)

add_executable(${PROJECT_NAME} main.cpp)

if(TARGET ukui-voice-wing-translations)
    add_dependencies(${PROJECT_NAME} ukui-voice-wing-translations)
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../log>
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        UKUI_VOICE_WING_TRANSLATIONS_DIR="${CMAKE_INSTALL_FULL_DATADIR}/ukui-voice-wing/translations"
        UKUI_VOICE_WING_BUILD_TRANSLATIONS_DIR="${CMAKE_BINARY_DIR}/translations"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_NAME}-core
        Qt5::Core
        Qt5::Gui
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
