project(kba)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core DBus REQUIRED)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
    )

set(bin_SRCS
    ${bin_SRCS}
    src/main.cpp
    src/serviceinterface.h
    src/serviceinterface.cpp
    src/securityquestionanswer.h
    src/securityquestionanswer.cpp
    )

link_directories(${LIBRARY_OUTPUT_PATH})

add_executable(kpr-backend ${bin_SRCS})
target_link_libraries(kpr-backend Qt5::Core Qt5::DBus -lkba -ljson-c)

install(TARGETS kpr-backend DESTINATION bin)
install(FILES org.kylin.KprBackend.conf DESTINATION /etc/dbus-1/system.d/)
install(FILES org.kylin.KprBackend.service DESTINATION /usr/share/dbus-1/system-services/)
