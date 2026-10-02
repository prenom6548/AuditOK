set(NSTACKX_ROOT "${DSOFTBUS_ROOT}/foundation/communication/dsoftbus/components/nstackx")

add_definitions(
        "-DMBEDTLS_INCLUDED"
        "-DNSTACKX_WITH_HMOS_LINUX"
)

set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "include"
        "interface"
        "platform/unix"
        "${NSTACKX_ROOT}/nstackx_core/"
        "${DSOFTBUS_ROOT}/utils/native/base/include/"
        "${DSOFTBUS_ROOT}/third_party/mbedtls/include"
        "${DSOFTBUS_ROOT}/third_party/mbedtls/include/mbedtls/"
)

set(SOURCES
        "core/nstackx_dev.c"
        "core/nstackx_event.c"
        "core/nstackx_log.c"
        "core/nstackx_mbedtls.c"
        "core/nstackx_socket.c"
        "core/nstackx_timer.c"
        "core/nstackx_util.c"
        "platform/unix/sys_dev.c"
        "platform/unix/sys_epoll.c"
        "platform/unix/sys_event.c"
        "platform/unix/sys_log.c"
        "platform/unix/sys_socket.c"
        "platform/unix/sys_timer.c"
        "platform/unix/sys_util.c"
        )


add_library(
        nstackx_util.open
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(nstackx_util.open PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        nstackx_util.open
        mbedtls_shared
        libsec_shared
)
