cmake_minimum_required(VERSION 3.5)

project(ukui-audio-service LANGUAGES CXX)
include(CheckIncludeFileCXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0 libxml-2.0)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(PA REQUIRED libpulse libpulse-mainloop-glib)
pkg_check_modules(PW REQUIRED libpipewire-0.3)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(PUGIXML REQUIRED pugixml)
pkg_check_modules(CANBERRA REQUIRED libcanberra)

include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${ALSA_INCLUDE_DIRS})
include_directories(${PA_INCLUDE_DIRS})
include_directories(${PW_INCLUDE_DIRS})
include_directories(${DBUS_INCLUDE_DIRS})
include_directories(${PUGIXML_INCLUDE_DIRS})
include_directories(${CANBERRA_INCLUDE_DIRS})

set(SRC
    ${PROJECT_SOURCE_DIR}/IJson.h
    ${PROJECT_SOURCE_DIR}/InitVolumeModule.cpp
    ${PROJECT_SOURCE_DIR}/InitVolumeModule.h
    ${PROJECT_SOURCE_DIR}/DeviceManagerModule.cpp
    ${PROJECT_SOURCE_DIR}/DeviceManagerModule.h
    ${PROJECT_SOURCE_DIR}/CustomAudioVolumeJson.cpp
    ${PROJECT_SOURCE_DIR}/CustomAudioVolumeJson.h
    ${PROJECT_SOURCE_DIR}/RestoreVolumeJson.cpp
    ${PROJECT_SOURCE_DIR}/RestoreVolumeJson.h
    ${PROJECT_SOURCE_DIR}/ISetting.h
    ${PROJECT_SOURCE_DIR}/AudioSetting.cpp
    ${PROJECT_SOURCE_DIR}/AudioSetting.h
    ${PROJECT_SOURCE_DIR}/GlobalThemeSettings.cpp
    ${PROJECT_SOURCE_DIR}/GlobalThemeSettings.h
    ${PROJECT_SOURCE_DIR}/SoundThemePlayerSetting.cpp
    ${PROJECT_SOURCE_DIR}/SoundThemePlayerSetting.h
    ${PROJECT_SOURCE_DIR}/SessionManagerSetting.cpp
    ${PROJECT_SOURCE_DIR}/SessionManagerSetting.h
    ${PROJECT_SOURCE_DIR}/SoundThemeModule.cpp
    ${PROJECT_SOURCE_DIR}/SoundThemeModule.h
    ${PROJECT_SOURCE_DIR}/Util.cpp
    ${PROJECT_SOURCE_DIR}/Util.h
    ${PROJECT_SOURCE_DIR}/PaDataType.cpp
    ${PROJECT_SOURCE_DIR}/PaDataType.h
    ${PROJECT_SOURCE_DIR}/BaseType.h
    ${PROJECT_SOURCE_DIR}/BackendFactory.h
    ${PROJECT_SOURCE_DIR}/PulseaudioBackendFactory.cpp
    ${PROJECT_SOURCE_DIR}/PulseaudioBackendFactory.h
    ${PROJECT_SOURCE_DIR}/PipewireBackendFactory.cpp
    ${PROJECT_SOURCE_DIR}/PipewireBackendFactory.h
    ${PROJECT_SOURCE_DIR}/IBackend.h
    ${PROJECT_SOURCE_DIR}/PulseaudioBackend.cpp
    ${PROJECT_SOURCE_DIR}/PulseaudioBackend.h
    ${PROJECT_SOURCE_DIR}/PipewireBackend.cpp
    ${PROJECT_SOURCE_DIR}/PipewireBackend.h
    ${PROJECT_SOURCE_DIR}/VolumeControl.cpp
    ${PROJECT_SOURCE_DIR}/VolumeControl.h
    ${PROJECT_SOURCE_DIR}/ManagerFactory.h
    ${PROJECT_SOURCE_DIR}/PulseaudioManagerFactory.cpp
    ${PROJECT_SOURCE_DIR}/PulseaudioManagerFactory.h
    ${PROJECT_SOURCE_DIR}/PipewireManagerFactory.cpp
    ${PROJECT_SOURCE_DIR}/PipewireManagerFactory.h
    ${PROJECT_SOURCE_DIR}/AudioManager.cpp
    ${PROJECT_SOURCE_DIR}/AudioManager.h
    ${PROJECT_SOURCE_DIR}/IStream.h
    ${PROJECT_SOURCE_DIR}/PaSinkStream.cpp
    ${PROJECT_SOURCE_DIR}/PaSinkStream.h
    ${PROJECT_SOURCE_DIR}/PaSourceStream.cpp
    ${PROJECT_SOURCE_DIR}/PaSourceStream.h
    ${PROJECT_SOURCE_DIR}/PaSinkInputStream.cpp
    ${PROJECT_SOURCE_DIR}/PaSinkInputStream.h
    ${PROJECT_SOURCE_DIR}/PaSourceOutputStream.cpp
    ${PROJECT_SOURCE_DIR}/PaSourceOutputStream.h
    ${PROJECT_SOURCE_DIR}/PulseaudioManager.cpp
    ${PROJECT_SOURCE_DIR}/PulseaudioManager.h
    ${PROJECT_SOURCE_DIR}/PipewireAlsaNode.cpp
    ${PROJECT_SOURCE_DIR}/PipewireAlsaNode.h
    ${PROJECT_SOURCE_DIR}/PipewireClient.cpp
    ${PROJECT_SOURCE_DIR}/PipewireClient.h
    ${PROJECT_SOURCE_DIR}/PipewireDevice.cpp
    ${PROJECT_SOURCE_DIR}/PipewireDevice.h
    ${PROJECT_SOURCE_DIR}/PipewireLink.cpp
    ${PROJECT_SOURCE_DIR}/PipewireLink.h
    ${PROJECT_SOURCE_DIR}/PipewireMetadata.cpp
    ${PROJECT_SOURCE_DIR}/PipewireMetadata.h
    ${PROJECT_SOURCE_DIR}/PipewireNode.cpp
    ${PROJECT_SOURCE_DIR}/PipewireNode.h
    ${PROJECT_SOURCE_DIR}/PipewireNodeListmodel.cpp
    ${PROJECT_SOURCE_DIR}/PipewireNodeListmodel.h
    ${PROJECT_SOURCE_DIR}/PipewirePort.cpp
    ${PROJECT_SOURCE_DIR}/PipewirePort.h
    ${PROJECT_SOURCE_DIR}/PipewireProfiler.cpp
    ${PROJECT_SOURCE_DIR}/PipewireProfiler.h
    ${PROJECT_SOURCE_DIR}/PipewireSettings.cpp
    ${PROJECT_SOURCE_DIR}/PipewireSettings.h
    ${PROJECT_SOURCE_DIR}/PipewireManager1.cpp
    ${PROJECT_SOURCE_DIR}/PipewireManager1.h
    ${PROJECT_SOURCE_DIR}/PipewireManager.cpp
    ${PROJECT_SOURCE_DIR}/PipewireManager.h
    ${PROJECT_SOURCE_DIR}/utils.cpp
    ${PROJECT_SOURCE_DIR}/utils.h
    ${PROJECT_SOURCE_DIR}/SystemdService.cpp
    ${PROJECT_SOURCE_DIR}/SystemdService.h
    ${PROJECT_SOURCE_DIR}/DbusServer.h
    ${PROJECT_SOURCE_DIR}/DbusServer.cpp
    ${PROJECT_SOURCE_DIR}/AudioMethod.h
    ${PROJECT_SOURCE_DIR}/AudioMethod.cpp
    ${PROJECT_SOURCE_DIR}/AudioContext.h
    ${PROJECT_SOURCE_DIR}/AudioContext.cpp
    ${PROJECT_SOURCE_DIR}/MediaControler.cpp
    ${PROJECT_SOURCE_DIR}/MediaControler.h
    ${PROJECT_SOURCE_DIR}/MediaAutoPauseModule.cpp
    ${PROJECT_SOURCE_DIR}/MediaAutoPauseModule.h
    ${PROJECT_SOURCE_DIR}/MultiAudioCombineJson.cpp
    ${PROJECT_SOURCE_DIR}/MultiAudioCombineJson.h
    ${PROJECT_SOURCE_DIR}/MultiAudioCombineModule.cpp
    ${PROJECT_SOURCE_DIR}/MultiAudioCombineModule.h
    ${PROJECT_SOURCE_DIR}/SingleApplication.cpp
    ${PROJECT_SOURCE_DIR}/SingleApplication.h
    ${PROJECT_SOURCE_DIR}/JackDetect.cpp
    ${PROJECT_SOURCE_DIR}/JackDetect.h
    ${PROJECT_SOURCE_DIR}/SoundThemePlayer.cpp
    ${PROJECT_SOURCE_DIR}/SoundThemePlayer.h
    ${PROJECT_SOURCE_DIR}/simple_logger/DateTime.h
    ${PROJECT_SOURCE_DIR}/simple_logger/Formatter.h
    ${PROJECT_SOURCE_DIR}/simple_logger/Logger.h
    ${PROJECT_SOURCE_DIR}/simple_logger/DateTime.cpp
    ${PROJECT_SOURCE_DIR}/simple_logger/Formatter.cpp
    ${PROJECT_SOURCE_DIR}/simple_logger/Logger.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/simple_logger)

add_executable(ukui-audio-service main.cpp ${SRC})

target_link_libraries(${PROJECT_NAME}
    ${GLIB2_LIBRARIES}
    ${ALSA_LIBRARIES}
    ${PA_LIBRARIES}
    ${PW_LIBRARIES}
    ${DBUS_LIBRARIES}
    ${PUGIXML_LIBRARIES}
    ${CANBERRA_LIBRARIES}
    -lpthread
    )

set(CMAKE_INSTALL_LIBDIR /usr/bin)
install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
