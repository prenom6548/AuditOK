add_definitions()

include_directories(
        .
)

set(SOURCES
        "cJSON.c"
        )

add_library(
        cjson_static
        STATIC
        ${SOURCES}
)

add_library(
        cjson
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
