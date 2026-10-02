project(fault_diagnosis_test LANGUAGES CXX)

set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g")

find_package(Qt5 COMPONENTS REQUIRED Widgets DBus)

set(SRCS
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/complete_machine_test.cpp)

include_directories(${SRC_DIR})

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::DBus)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/share/kylin-os-manager/fault-diagnosis/test/)
install(FILES solution_key.db DESTINATION /usr/share/kylin-os-manager/fault-diagnosis/test/)
