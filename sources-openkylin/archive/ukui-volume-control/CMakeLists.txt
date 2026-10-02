cmake_minimum_required(VERSION 3.22)
project(ukui-volume-control)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Handle the Qt rcc code generator automatically.
set(CMAKE_AUTORCC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

add_subdirectory(audio)
add_subdirectory(backend)
add_subdirectory(tray)
add_subdirectory(ukui-shortcut-audio)

set(SERVICE_FILES data/ukui-volume-control.service)
install(FILES ${SERVICE_FILES} DESTINATION "/usr/lib/systemd/user/")
