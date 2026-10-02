set(APPABILITY_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})
set(calendarcode 
    "${APPABILITY_TOP_DIR}/libkycalendar.h"
    "${APPABILITY_TOP_DIR}/libkycalendar.c")

set(audiocode 
    "${APPABILITY_TOP_DIR}/libkyaudio.h"
    "${APPABILITY_TOP_DIR}/libkyaudio.c")

set(videocode 
    "${APPABILITY_TOP_DIR}/libkyvideo.h"
    "${APPABILITY_TOP_DIR}/libkyvideo.c")

add_library(kycalendar SHARED ${calendarcode})
add_library(kyaudio SHARED ${audiocode})
add_library(kyvideo SHARED ${videocode})

set_target_properties(kyvideo  PROPERTIES VERSION 3.0.0 SOVERSION 1)
set_target_properties(kyaudio  PROPERTIES VERSION 3.0.0 SOVERSION 1)
set_target_properties(kycalendar  PROPERTIES VERSION 3.0.0 SOVERSION 1)

add_executable(kyvideo-test test/kyvideo-test.c)
add_executable(kyaudio-test test/kyaudio-test.c)
add_executable(kycalendar-test test/kycalendar-test.c)

find_library(DBUS_LIB dbus-1)
target_link_libraries(kyvideo ${DBUS_LIB} kylog)
target_link_libraries(kyvideo-test kyvideo)

target_link_libraries(kyaudio ${DBUS_LIB} kylog)
target_link_libraries(kyaudio-test kyaudio)

target_link_libraries(kycalendar ${DBUS_LIB})
target_link_libraries(kycalendar-test kycalendar)

# 包含 GNU 安装目录设置  
include(GNUInstallDirs)

# 设置 CMAKE_INSTALL_LIBDIR 变量  
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv8l")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/arm-linux-gnueabihf")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/mips64el-linux-gnuabi64")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/i386-linux-gnu")
else()
        set(CMAKE_INSTALL_LIBDIR "/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu")
endif()
# 安装库文件  
install(TARGETS kyvideo kyaudio kycalendar
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(TARGETS kyrealtime
#         DESTINATION lib/kysdk/kysdk-system)

install(FILES libkyvideo.h libkyaudio.h libkycalendar.h
        DESTINATION include/kysdk/kysdk-system)