# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Gui DBus Widgets Svg REQUIRED)

# 包含 GTest 库和 pthread 库
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(GIOUNIX2 REQUIRED gio-unix-2.0)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 开启代码覆盖率相关编译选项（对应QMAKE_LFLAGS和QMAKE_CXXFLAGS中代码覆盖率相关设置）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage --coverage -fno-inline -fno-access-control")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

# 定义源文件列表，对应原来的SOURCES变量
set(SOURCES
    main.cpp
    ../../bioauth/src/uniauthservice.cpp
    ../../bioauth/src/biotypes.cpp
    ../../bioauth/src/giodbus.cpp
    ../../bioauth/src/bioauth.cpp
    ../../bioauth/src/biodevices.cpp
    ../../bioauth/src/loginoptionswidget.cpp
)

# 定义头文件列表，对应原来的HEADERS变量
set(HEADERS
    ../../bioauth/include/uniauthservice.h
    ../../bioauth/include/bioauth.h
    ../../bioauth/include/biodevices.h
    ../../bioauth/include/loginoptionswidget.h
)

# 包含头文件的路径设置，对应原来的INCLUDEPATH变量
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../kt-test-utils/cpp-stub
    ${CMAKE_CURRENT_SOURCE_DIR}/../kt-test-utils/cpp-stub-ext
    ../../bioauth/include
    ../../common
    ${GIOUNIX2_INCLUDE_DIRS}
)

# 使用qt5_wrap_cpp生成元对象代码相关的源文件
qt5_wrap_cpp(MOC_SOURCES ${HEADERS})

# 添加可执行文件或库目标，将元对象代码源文件一起添加进去
add_executable(unit_test_biodevices ${SOURCES} ${MOC_SOURCES})

# 链接Qt相关的库
target_link_libraries(unit_test_biodevices
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    Qt5::Widgets
    Qt5::Svg
    ${GIOUNIX2_LIBRARIES}
)

# 链接 GTest 库
target_link_libraries(unit_test_biodevices
    GTest::GTest
    GTest::Main
    Threads::Threads
)

