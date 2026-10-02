set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Sql Test Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Sql Test Widgets REQUIRED)

include_directories(..)
include_directories(../../libsearch/appdata)

add_executable(appDbUtilsTest
    app-db-utils-test.cpp
    ../app-db-utils.cpp
)

target_include_directories(appDbUtilsTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_SOURCE_DIR}/libsearch
    ${CMAKE_SOURCE_DIR}/libsearch/appdata
)

target_link_libraries(appDbUtilsTest PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Test
    ukui-search
)

add_test(NAME appDbUtilsTest COMMAND appDbUtilsTest)

add_executable(pendingAppInfoTest
        pending-app-info-test.cpp)
target_link_libraries(pendingAppInfoTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test)
add_test(pendingAppInfoTest ${CMAKE_BINARY_DIR}/ukui-search-app-data-service/autotest/pendingAppInfoTest)

add_executable(pendingAppInfoQueueTest
        pending-app-info-queue-test.cpp
        ../pending-app-info-queue.cpp
        ../app-db-manager.h
        app-db-manager-test-stub.cpp)
target_include_directories(pendingAppInfoQueueTest PRIVATE
        ../../libsearch/filesystemwatcher)
target_link_libraries(pendingAppInfoQueueTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        ukui-search)
add_test(pendingAppInfoQueueTest ${CMAKE_BINARY_DIR}/ukui-search-app-data-service/autotest/pendingAppInfoQueueTest)
set_tests_properties(pendingAppInfoQueueTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.*=false;QT_FATAL_WARNINGS=0")

add_executable(signalTransformerTest
        signal-transformer-test.cpp
        ../signal-transformer.cpp
        ../app-db-manager.h
        app-db-manager-test-stub.cpp)
target_include_directories(signalTransformerTest PRIVATE
        ../../libsearch/filesystemwatcher)
target_link_libraries(signalTransformerTest PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Test
        Qt${QT_VERSION_MAJOR}::Widgets
        ukui-search)
add_test(signalTransformerTest ${CMAKE_BINARY_DIR}/ukui-search-app-data-service/autotest/signalTransformerTest)
set_tests_properties(signalTransformerTest PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.*=false;QT_FATAL_WARNINGS=0")
