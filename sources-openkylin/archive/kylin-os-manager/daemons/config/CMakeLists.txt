cmake_minimum_required(VERSION 3.16)

project(komd-config LANGUAGES CXX)

set(CONFIG_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)

add_executable(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

find_package(Qt5 COMPONENTS Core REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)

set(SRCS "${CONFIG_DIR}/main.cpp"
         "${CONFIG_DIR}/utils.cpp"
         "${CONFIG_DIR}/utils-config.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CONFIG_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/lib/kylin-os-manager/bin/)
install(FILES ${CONFIG_DIR}/komd-config.desktop DESTINATION /etc/xdg/autostart)