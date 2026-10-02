add_definitions()

include_directories(
        "include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
)

set(SOURCES
        "ipc_center.cpp"
        "ipc_object_stub.cpp"
        "ipc_skeleton.cpp"
        "iremote_object.cpp"
        "message_parcel.cpp"
        )

add_library(
        ipc_single
        SHARED
        ${SOURCES}
)
target_link_directories(ipc_single PUBLIC /opt/kylin-connectivity/dsoftbus/)

target_link_libraries(
        ipc_single
        utils
)

add_library(
        ipc_core
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)

target_link_libraries(
        ipc_core
        utils
)