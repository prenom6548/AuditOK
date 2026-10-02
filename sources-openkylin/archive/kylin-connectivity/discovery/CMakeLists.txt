set(KDISCOVERY_SOURCES
        discovery.cpp
        discovery.h
        discdeviceinfo.cpp
        discdeviceinfo.h
        tlv.h
        tlv.cpp
        unix-util.h
)

add_library(discovery ${KDISCOVERY_SOURCES})

target_include_directories(discovery PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/discserver)
target_include_directories(discovery PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(discovery PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        common
)
