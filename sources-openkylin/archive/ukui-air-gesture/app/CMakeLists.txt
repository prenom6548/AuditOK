cmake_minimum_required(VERSION 3.8)

project(ukui-air-gesture)

cmake_policy(SET CMP0071 NEW)
include(GNUInstallDirs)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets DBus)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

pkg_check_modules(PC_LIBS REQUIRED kysdk-waylandhelper kysdk-ukuiwindowhelper gsettings-qt libevdev)

qt5_add_resources(RESOURCE_FILES resources.qrc)

# Python文件和模型等安装路径设置
set(AIR_GESTURE_LIBEXEC_DIR ${CMAKE_INSTALL_FULL_LIBEXECDIR}/${PROJECT_NAME})
set(AIR_GESTURE_PYTHON_DIR ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/python)
set(AIR_GESTURE_MODEL_DIR ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/models)
set(AIR_GESTURE_VENV_DIR ${AIR_GESTURE_LIBEXEC_DIR}/venv)

add_custom_command(
    OUTPUT 
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-window-management.h
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-window-management.cpp
    COMMAND wayland-scanner++ 
        ${CMAKE_CURRENT_SOURCE_DIR}/proto/ukui-window-management.xml
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-window-management.h
        ${CMAKE_CURRENT_BINARY_DIR}/ukui-window-management.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/proto/ukui-window-management.xml
)

add_executable(ukui-air-gesture
    src/main.cpp
    src/dbusmanager.cpp
    src/singleapplication.cpp
    src/dbusadaptor.cpp
    src/gestureactionexecutor.cpp
    src/mediacontroller.cpp
    src/pythonprocessmanager.cpp
    src/mainwindow.cpp
    include/dbusmanager.h
    include/singleapplication.h
    include/dbusadaptor.h
    include/gestureactionexecutor.h
    include/mediacontroller.h
    include/pythonprocessmanager.h
    include/mainwindow.h
    ${CMAKE_CURRENT_BINARY_DIR}/ukui-window-management.cpp
    ${RESOURCE_FILES}
)

target_include_directories(ukui-air-gesture PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/include
    ${PC_LIBS_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(ukui-air-gesture PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::DBus
    ${PC_LIBS_LIBRARIES}
    wayland-client++
    ukui-log4qt
)

# 将安装路径作为编译定义传递给源代码
target_compile_definitions(ukui-air-gesture PRIVATE
    AIR_GESTURE_PYTHON_MAIN="${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/python/main.py"
    AIR_GESTURE_VENV_PYTHON="${CMAKE_INSTALL_FULL_LIBEXECDIR}/${PROJECT_NAME}/venv/bin/python"
)

install(TARGETS ukui-air-gesture
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES org.ukui.air.gesture.desktop
    DESTINATION /etc/xdg/autostart
)

# 安装Python脚本和模型文件
install(DIRECTORY python/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/python
    FILES_MATCHING
    PATTERN "*.py"
    PATTERN "requirements.txt"
)

install(FILES python/hand_landmarker.task
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/models
)

# 安装后脚本：创建虚拟环境并安装依赖
install(CODE "
    set(_venv_path \"\$ENV{DESTDIR}${AIR_GESTURE_VENV_DIR}\")
    set(_requirements \"\$ENV{DESTDIR}${AIR_GESTURE_PYTHON_DIR}/requirements.txt\")

    execute_process(
        COMMAND \"${Python3_EXECUTABLE}\" -m venv \"\${_venv_path}\"
        RESULT_VARIABLE _venv_result
    )
    if(NOT _venv_result EQUAL 0)
        message(FATAL_ERROR \"Failed to create virtual environment at \${_venv_path}\")
    endif()

    execute_process(
            COMMAND \"\${_venv_path}/bin/python\" -m pip install --no-cache-dir
                    -i https://pypi.tuna.tsinghua.edu.cn/simple
                    --trusted-host pypi.tuna.tsinghua.edu.cn
                    --default-timeout 120
                    --retries 10
                    -r \"\${_requirements}\"
        RESULT_VARIABLE _pip_result
    )
    if(NOT _pip_result EQUAL 0)
        message(FATAL_ERROR \"Failed to install Python requirements from \${_requirements}\")
    endif()
")
