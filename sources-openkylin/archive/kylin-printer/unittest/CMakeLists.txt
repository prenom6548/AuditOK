cmake_minimum_required(VERSION 3.5)
#Set Project Name
project(kylin-printer-unittest LANGUAGES CXX)
#message(${ PROJECT_NAME })
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-w)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Core DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core DBus REQUIRED)

#Gsetting
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(QGSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)

#add ukui log
file(GLOB_RECURSE UKUI_LOG4QT "/usr/lib/libukui-log4qt.so.1.0.0")

#add include 
include_directories("../printer_manager/")
include_directories("../common/")

file(GLOB printer_manager_SRC 
    "../printer_manager/ukuiPrinter.cpp"
    "../printer_manager/ukuiPrinter.h"
    "../common/common.cpp"
    "../common/common.h"
)

add_executable(kylin-printer-unittest
    main.cpp
    ${printer_manager_SRC}
)

target_link_libraries(kylin-printer-unittest Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::DBus )

#dependencies
target_link_libraries(kylin-printer-unittest "-lpthread -lz")
#cups
target_link_libraries(kylin-printer-unittest "-lcups")

target_link_libraries(kylin-printer-unittest ${UKUI_LOG4QT})
#target_link_libraries(kylin-printer PkgConfig::QGSETTINGS)
# ########### install ###############
# install(TARGETS kylin-printer DESTINATION /usr/bin/)
# install(FILES ${PROJECT_SOURCE_DIR}/kylin-printer.desktop DESTINATION /etc/xdg/autostart/)
# install(FILES ${PROJECT_SOURCE_DIR}/ui/data/org.ukui.log4qt.kylin-printer.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
