set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Test DBus Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test DBus Widgets)

add_executable(recentFilesServiceTest
        recent-files-service-test.cpp)

target_include_directories(recentFilesServiceTest PRIVATE
        ..
        ${CMAKE_SOURCE_DIR}/framework/config
        /usr/include/ukui-file-metadata
)

target_link_libraries(recentFilesServiceTest PRIVATE
        ukui-task-manager
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
        ukui-file-metadata
)

add_test(NAME recentFilesServiceTest COMMAND $<TARGET_FILE:recentFilesServiceTest>)
set_tests_properties(recentFilesServiceTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
