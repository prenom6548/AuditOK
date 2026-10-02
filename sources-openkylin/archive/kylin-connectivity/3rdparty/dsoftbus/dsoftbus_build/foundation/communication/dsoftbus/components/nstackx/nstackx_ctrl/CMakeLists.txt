add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "interface"
        "${DSOFTBUS_ROOT}/third_party/libcoap/include/coap2"
        "${DSOFTBUS_ROOT}/third_party/cJSON" ,
        "${DSOFTBUS_ROOT}/third_party/bounds_checking_function/include"
        "include"
        "include/coap_discover"
        "../nstackx_util/interface"
        "../nstackx_util/platform/unix"
)

set(SOURCES
        "core/coap_discover/coap_app.c"
        "core/coap_discover/coap_client.c"
        "core/coap_discover/coap_discover.c"
        "core/coap_discover/json_payload.c"
        "core/nstackx_common.c"
        "core/nstackx_database.c"
        "core/nstackx_device.c"
        "core/nstackx_smartgenius.c"
        )


add_library(
        nstackx_ctrl
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(nstackx_ctrl PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        nstackx_ctrl
        nstackx_util.open
        cjson_static
        libcoap
        libsec_shared
)

