message(STATUS "Building ukui-voice-wing tests")

find_package(Qt5 COMPONENTS Core Gui Test REQUIRED)

add_executable(test_voice_input test_voice_input.cpp)

set_target_properties(test_voice_input PROPERTIES AUTOMOC ON)

target_include_directories(test_voice_input PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
)

target_link_libraries(test_voice_input PRIVATE
    ${PROJECT_NAME}-core
    Qt5::Core
    Qt5::Gui
    Qt5::Test
)

add_test(NAME test_voice_input COMMAND test_voice_input)

set_tests_properties(test_voice_input PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
