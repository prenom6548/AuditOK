add_definitions()

include_directories(
        "include"
)

set(SOURCES
        "src/refbase.cpp"
        "src/parcel.cpp"
        )

add_library(
        utils
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(utils PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(utils libsec_shared)
