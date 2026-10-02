add_definitions()

include_directories(
        "include"
)

set(SOURCES
        "main.cpp"
        )
#
find_library (
        log-lib
        log
)


add_library(
        libhilog
        SHARED
        ${SOURCES}
#        ${log-lib}
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)