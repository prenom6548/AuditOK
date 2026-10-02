cmake_minimum_required(VERSION 3.16)
set(TARGET ondevice-engine-plugin)
project(ondevice-engine-plugin)

find_package(KylinAiEngine CONFIG REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

set(ONDEVICE_SOURCE
    ondeviceengine.h
    ondeviceengine.cpp
)

kylin_ai_add_engine_plugin(${TARGET} AI_ENGINE
    ${ONDEVICE_SOURCE}
)

set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${TARGET}
    kylin-ondevice-nlp-engine
    kylin-ondevice-vision-engine
    kylin-ondevice-speech-engine
)

install(FILES ondeviceengine.h
    DESTINATION 
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})