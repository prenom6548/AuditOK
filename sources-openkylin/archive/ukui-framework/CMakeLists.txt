cmake_minimum_required(VERSION 3.5)
find_package(PkgConfig)
project(ukui-framework VERSION 1.0)

add_subdirectory(dbus)
add_subdirectory(backup-service)
