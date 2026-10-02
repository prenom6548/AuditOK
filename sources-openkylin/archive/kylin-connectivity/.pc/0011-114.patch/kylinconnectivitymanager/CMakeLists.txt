cmake_minimum_required(VERSION 3.16)

project(kylin-connectivity-manager LANGUAGES CXX)

set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SRCS
    "${TOP_DIR}/connectivitymanager.cpp"
    "${TOP_DIR}/connectivitymanager.cpp"
    "${TOP_DIR}/main.cpp"
   )

add_executable(${PROJECT_NAME} ${SRCS})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

find_package(Qt5 REQUIRED COMPONENTS Core)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)

find_package(Qt5 REQUIRED COMPONENTS DBus)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${TOP_DIR}/com.kylin.connectivity.manager.conf DESTINATION /etc/dbus-1/system.d/)