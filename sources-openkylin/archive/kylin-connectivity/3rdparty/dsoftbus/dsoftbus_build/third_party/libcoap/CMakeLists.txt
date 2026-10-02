add_definitions(
        "-DSTANDARD_SYSTEM_ENABLE" )

include_directories(
        "include/coap2"
)

set(SOURCES
        "src/address.c"
        "src/async.c"
        "src/block.c"
        "src/coap_debug.c"
        "src/coap_event.c"
        "src/coap_hashkey.c"
        "src/coap_io.c"
        "src/coap_notls.c"
        "src/coap_openssl.c"
        "src/coap_session.c"
        "src/coap_time.c"
        "src/encode.c"
        "src/mem.c"
        "src/net.c"
        "src/option.c"
        "src/pdu.c"
        "src/resource.c"
        "src/str.c"
        "src/subscribe.c"
        "src/uri.c"
        )

add_library(
        libcoap_static
        STATIC
        ${SOURCES}
)

add_library(
        libcoap
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)