# CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

find_package(Qt5 COMPONENTS Core Multimedia Gui DBus REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GIOUNIX2 REQUIRED gio-unix-2.0)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)

# 包含 GTest 库和 pthread 库
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(KF5IdleTime)
#find_package(Qt5X11Extras REQUIRED)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 开启代码覆盖率相关编译选项（对应QMAKE_LFLAGS和QMAKE_CXXFLAGS中代码覆盖率相关设置）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage --coverage -fno-inline -fno-access-control")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

file(GLOB SOURCES "../../tools/*.cpp")
file(GLOB HEADERS "../../tools/*.h")

set(SOURCES
    ../../tools/lockchecker.cpp
    ../../tools/loginedusers.cpp
    ../../ukui-session/ukuilockinfo.cpp
    ../../ukui-session/xdgdesktopfile.cpp
    ../../ukui-session/xdgdirs.cpp
    unit_test_lockchecker.cpp
)
set(HEADERS
    ../../tools/lockchecker.h
    ../../ukui-session/ukuilockinfo.h
    ../../ukui-session/xdgdesktopfile.h
    ../../ukui-session/xdgdirs.h
    ../../tools/loginedusers.h
)


pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
include_directories(${GSETTINGS_QT_INCLUDES})
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO2 REQUIRED gio-2.0)

pkg_search_module(X11 REQUIRED x11)
pkg_check_modules(KDKINFO REQUIRED kysdk-sysinfo)

include_directories(${X11_INCLUDES})
include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${GIO2_INCLUDE_DIRS})
include_directories(${KDKINFO_INCLUDE_DIRS})

# 包含头文件的路径设置，对应原来的INCLUDEPATH变量
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../kt-test-utils/cpp-stub
    ${CMAKE_CURRENT_SOURCE_DIR}/../kt-test-utils/cpp-stub-ext
)

# 使用qt5_wrap_cpp生成元对象代码相关的源文件
qt5_wrap_cpp(MOC_SOURCES ${HEADERS})

add_executable(unit_test_lockchecker ${SOURCES} ${MOC_SOURCES})

target_link_libraries(unit_test_lockchecker
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    KF5::IdleTime
    Qt5::Multimedia
    ${GSETTINGS_QT_LIBRARIES} ${X11_LIBRARIES}
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
    ${KDKINFO_LIBRARIES}
    #Qt5::X11Extras
    -lX11
)

# 链接 GTest 库
target_link_libraries(unit_test_lockchecker
    GTest::GTest
    GTest::Main
    Threads::Threads
)

