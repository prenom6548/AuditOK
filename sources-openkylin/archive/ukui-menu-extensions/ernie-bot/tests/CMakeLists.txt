set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets Quick Network Test Qml REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets Quick Network Test Qml REQUIRED)

include_directories(../src)

set(TEST_RUNTIME_OUTPUT_DIR "${CMAKE_BINARY_DIR}/ernie-bot/tests")

add_executable(ernieBotTest
    ../src/ernie-bot.cpp
    ../src/message.cpp
    ../src/message-action.cpp
    ../src/message-model.cpp
    ../src/session.cpp
    ../src/session-manager.cpp
    ../src/chat.h
    chat-stub.cpp
    ernie-bot-test.cpp)

add_executable(chatTest
    ../src/chat.cpp
    chat-test.cpp)

add_executable(messageActionTest
    ../src/message-action.cpp
    message-action-test.cpp)

add_executable(messageTest
    ../src/message.cpp
    ../src/message-action.cpp
    message-test.cpp)

add_executable(messageModelTest
    ../src/message.cpp
    ../src/message-action.cpp
    ../src/message-model.cpp
    ../src/session.cpp
    ../src/session-manager.cpp
    ../src/chat.h
    chat-stub.cpp
    message-model-test.cpp)

add_executable(sessionTest
    ../src/message.cpp
    ../src/message-action.cpp
    ../src/session.cpp
    ../src/chat.h
    chat-stub.cpp
    session-test.cpp)

add_executable(sessionManagerTest
    ../src/message.cpp
    ../src/message-action.cpp
    ../src/session.cpp
    ../src/session-manager.cpp
    ../src/chat.h
    chat-stub.cpp
    session-manager-test.cpp)

set_target_properties(ernieBotTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${TEST_RUNTIME_OUTPUT_DIR}"
)

set_target_properties(chatTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${TEST_RUNTIME_OUTPUT_DIR}"
)

set_target_properties(messageActionTest messageTest messageModelTest sessionTest sessionManagerTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${TEST_RUNTIME_OUTPUT_DIR}"
)

target_compile_options(ernieBotTest PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(ernieBotTest PRIVATE -fprofile-arcs -ftest-coverage)
target_compile_options(chatTest PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(chatTest PRIVATE -fprofile-arcs -ftest-coverage)
target_compile_options(messageActionTest PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(messageActionTest PRIVATE -fprofile-arcs -ftest-coverage)
target_compile_options(messageTest PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(messageTest PRIVATE -fprofile-arcs -ftest-coverage)
target_compile_options(messageModelTest PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(messageModelTest PRIVATE -fprofile-arcs -ftest-coverage)
target_compile_options(sessionTest PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(sessionTest PRIVATE -fprofile-arcs -ftest-coverage)
target_compile_options(sessionManagerTest PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(sessionManagerTest PRIVATE -fprofile-arcs -ftest-coverage)
target_compile_definitions(chatTest PRIVATE BUILD_TEST)

find_package(ukui-menu REQUIRED)

target_link_libraries(ernieBotTest
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Test
    ukui-menu
)

target_link_libraries(chatTest
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Test
)

target_link_libraries(messageActionTest
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

target_link_libraries(messageTest
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

target_link_libraries(messageModelTest
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Test
)

target_link_libraries(sessionTest
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Test
)

target_link_libraries(sessionManagerTest
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME ernieBotTest COMMAND ${TEST_RUNTIME_OUTPUT_DIR}/ernieBotTest)
add_test(NAME chatTest COMMAND ${TEST_RUNTIME_OUTPUT_DIR}/chatTest)
add_test(NAME messageActionTest COMMAND ${TEST_RUNTIME_OUTPUT_DIR}/messageActionTest)
add_test(NAME messageTest COMMAND ${TEST_RUNTIME_OUTPUT_DIR}/messageTest)
add_test(NAME messageModelTest COMMAND ${TEST_RUNTIME_OUTPUT_DIR}/messageModelTest)
add_test(NAME sessionTest COMMAND ${TEST_RUNTIME_OUTPUT_DIR}/sessionTest)
add_test(NAME sessionManagerTest COMMAND ${TEST_RUNTIME_OUTPUT_DIR}/sessionManagerTest)
