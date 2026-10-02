project(uniauth-backend)

set(CMAKE_AUTOMOC ON)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
    )

set(bin_SRCS
    ${bin_SRCS}
    src/main.cpp
    src/biodeviceinfo.h
    src/biodeviceinfo.cpp
    src/serviceinterface.h
    src/serviceinterface.cpp
    src/servicemanager.h
    src/servicemanager.cpp
    src/personalizeddata.h
    src/CSingleton.h
    src/personalizeddata.cpp
    src/rsac.h
    src/rsac.cpp
    src/greeterconfiginterface.h
    src/greeterconfiginterface.cpp
    src/greeterconfig.h
    src/greeterconfig.cpp
    src/userconfigsettings.cpp
    src/userconfigsettings.h
    src/dbusaccesscontrol.h
    src/dbusaccesscontrol.cpp
    src/usersafeconfigpath.h
    src/usersafeconfigpath.cpp
    )

add_executable(uniauth-backend ${bin_SRCS})
target_link_libraries(uniauth-backend Qt5::Core Qt5::DBus -lukui-log4qt -lcrypto)

install(TARGETS uniauth-backend DESTINATION bin)
install(FILES org.ukui.UniauthBackend.conf DESTINATION /etc/dbus-1/system.d/)
install(FILES org.ukui.UniauthBackend.service DESTINATION /usr/share/dbus-1/system-services/)
install(FILES org.ukui.UniauthBackend.yaml DESTINATION /usr/share/libpam-biometric/kylin-config/)
