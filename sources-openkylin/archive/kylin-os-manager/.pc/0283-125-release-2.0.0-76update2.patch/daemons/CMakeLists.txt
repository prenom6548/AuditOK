cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -g")

add_subdirectory(config)
add_subdirectory(service)
add_subdirectory(sessionservice)
add_subdirectory(kylin-core-dump-monitor)
add_subdirectory(kylin-os-manager-driver-acquirer)
