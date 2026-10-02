set(CMAKE_AUTOMOC ON)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus REQUIRED)

add_library(inotify-config-service MODULE
        inotify-config-service.cpp
        inotify-config-service.h
        inotify-config.cpp
        inotify-config.h
)

target_link_libraries(
        inotify-config-service PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        ukuiserviceinterface
)

install(TARGETS inotify-config-service DESTINATION /usr/libexec/ukui-service/system)
install(FILES conf/com.ukui.search.qt.systemdbus.service DESTINATION /usr/share/dbus-1/system-services/)
install(FILES conf/com.ukui.search.qt.systemdbus.conf DESTINATION /etc/dbus-1/system.d/)
install(FILES conf/com.ukui.search.qt.systemdbus.yaml DESTINATION /etc/kylin-config/basic/)