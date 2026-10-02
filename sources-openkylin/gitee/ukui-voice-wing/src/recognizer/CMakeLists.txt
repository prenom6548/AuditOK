message(STATUS "Building ukui-voice-wing recognizer")

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(KyaiSpeech REQUIRED kysdk-coreai-speech)

add_library(${PROJECT_NAME}-recognizer STATIC
    voiceToStr.cpp
    voiceToStr.h
)

set_target_properties(${PROJECT_NAME}-recognizer PROPERTIES
    AUTOMOC ON
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${PROJECT_NAME}-recognizer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${KyaiSpeech_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}-recognizer
    PUBLIC
        ${PROJECT_NAME}-config
        Qt5::Core
        ${PROJECT_NAME}-log
    PRIVATE
        ${KyaiSpeech_LIBRARIES}
)
