cmake_minimum_required(VERSION 3.5)

project(unit_test_kylin-printer)


set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")


option(UNIT_TEST "option for unit test" ON)
if (UNIT_TEST)
    add_definitions(-DUNIT_TEST)
endif(UNIT_TEST)

set (SOURCE_PATH ../..)

include(${SOURCE_PATH}/cmake/common.cmake)
include(${SOURCE_PATH}/cmake/printer.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")

add_definitions(-DTEST_DEBUG)
add_definitions(-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_definitions(-DPROJECT_BINARY_DIR=\"${PROJECT_BINARY_DIR}\")

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")
add_definitions(-DPROJECT_VERSION=\"${PROJECT_VERSION}\")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")


find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Network DBus REQUIRED)
include_directories(${SOURCE_PATH}/printer_manager/
                    ${SOURCE_PATH}/device_manager/
                    ${SOURCE_PATH}/ui/
                    ${SOURCE_PATH}/ui/custom_ui/
                    ${SOURCE_PATH}/ui/main_win_ui/
                    ${SOURCE_PATH}/ui/new_pop_window/
                    ${SOURCE_PATH}/util/
                    ${SOURCE_PATH}/dictionary/
                    ${SOURCE_PATH}/common/
                    ${SOURCE_PATH}/avahi_qt/
                    ${SOURCE_PATH}/qt_zeroconf/
                    ${SOURCE_PATH}/ui/new_property_window/
                    ${SOURCE_PATH}/ui/job_manager/
                    ${SOURCE_PATH}/ui/new_property_window/SecondComponent/
                    ${SOURCE_PATH}/include
                    ${SOURCE_PATH}/backend/
                    ${SOURCE_PATH}/backend/ui/
                    )

add_executable(${PROJECT_NAME}
    main.cpp
    unit_test_kylin-printer.cpp
)

target_link_kysdk_libraries(${PROJECT_NAME})
target_link_glib_libraries(${PROJECT_NAME})
target_link_snmp_libraries(${PROJECT_NAME})
target_link_qgsettings_libraries(${PROJECT_NAME})
target_link_cups_libraries(${PROJECT_NAME})
target_link_cupsfilters_libraries(${PROJECT_NAME})
target_link_avahi_libraries(${PROJECT_NAME})
target_link_usb_libraries(${PROJECT_NAME})
target_link_udev_libraries(${PROJECT_NAME})
target_link_KWindowSystem_libraries(${PROJECT_NAME})
target_link_smbclient_libraries(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)

target_link_libraries(${PROJECT_NAME} "-lpthread -lz")


target_link_libraries(${PROJECT_NAME} "-lgtest")
target_link_libraries(${PROJECT_NAME} "-lgtest_main")
target_link_libraries(${PROJECT_NAME} "-lgcov")

target_link_libraries(${PROJECT_NAME} kt-test-utils)
target_link_libraries(${PROJECT_NAME} lib-kylin-printer)
