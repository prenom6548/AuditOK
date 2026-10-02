# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.5)

#项目信息
project (kylin-updatefinish-notify-new)

#添加第三方函数库
find_package(PkgConfig)

#增加指定头文件
pkg_check_modules(GLib REQUIRED glib-2.0)
if(GLib_FOUND)
include_directories(${GLib_INCLUDE_DIRS})
link_libraries(${GLib_LIBRARIES})
endif(GLib_FOUND)

pkg_check_modules(DBus REQUIRED dbus-1)
if(DBus_FOUND)
include_directories(${DBus_INCLUDE_DIRS})
link_libraries(${DBus_LIBRARIES})
endif(DBus_FOUND)

pkg_check_modules(GObject REQUIRED gobject-2.0)
if(GObject_FOUND)
include_directories(${GObject_INCLUDE_DIRS})
link_libraries(${GObject_LIBRARIES})
endif(GObject_FOUND)

pkg_check_modules(Libnotify REQUIRED libnotify)
if(Libnotify_FOUND)
include_directories(${Libnotify_INCLUDE_DIRS})
link_libraries(${Libnotify_LIBRARIES})
endif(Libnotify_FOUND)

aux_source_directory(./src SRC_MAIN)

add_definitions("-Wall -g")

#指定生成目标
add_executable(kylin-updatefinish-notify-new ${SRC_MAIN})
#链接动态库
target_link_libraries(kylin-updatefinish-notify-new libglib-2.0.so libnotify.so libgobject-2.0.so)
#确定执行文件生成路径
set_target_properties(
kylin-updatefinish-notify-new
PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../"
)
