set(notificationClientTest_SRCS
        notification-client-test.cpp
        notification-client-test.h
        main.cpp notification-settings-test.cpp notification-settings-test.h)
include_directories(${CMAKE_SOURCE_DIR}/libukui-notification ${CMAKE_SOURCE_DIR}/libukui-notification/notification-settings)

add_executable(ukui-notification-test ${notificationClientTest_SRCS})

target_link_libraries(ukui-notification-test PRIVATE
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        ukui-notification
        )