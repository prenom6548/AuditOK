add_definitions()

include_directories(
        "include"
)

set(SOURCES
        "device_auth.c"
        )

add_library(
        deviceauth_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)