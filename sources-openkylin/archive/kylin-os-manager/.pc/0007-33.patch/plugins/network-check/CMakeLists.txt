cmake_minimum_required(VERSION 3.5)

project(NetWorkCheckTool LANGUAGES CXX)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

option(XiaMen_Version "option for XiaMen, include inner Net Check" OFF)

if (XiaMen_Version) 
	add_definitions(-DXiaMen_Version)
endif(XiaMen_Version)

add_subdirectory(libBase)
add_subdirectory(libNWDBus)
add_subdirectory(HWCheck)
add_subdirectory(IPCheck)
add_subdirectory(DHCPCheck)
add_subdirectory(DNSCheck)
add_subdirectory(HostCheck)
#add_subdirectory(ProxyCheck)
add_subdirectory(NetCheck)
add_subdirectory(appUI)
