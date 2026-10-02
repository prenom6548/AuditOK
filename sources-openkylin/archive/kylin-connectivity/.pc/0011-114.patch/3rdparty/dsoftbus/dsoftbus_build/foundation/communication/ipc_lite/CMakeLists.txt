#add_definitions("-DLITE_LINUX_BINDER_IPC")

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "${DSOFTBUS_ROOT}/foundation/communication/ipc_lite/interfaces/kits"
        "${DSOFTBUS_ROOT}/utils/native/lite/include"

        "frameworks/liteipc_linux/include"
        "frameworks/liteipc/include"
        "${DSOFTBUS_ROOT}/third_party/bounds_checking_function/include"

        ${common_log_inc}
)

set(SOURCES
        "frameworks/liteipc/src/serializer.c"
        "frameworks/liteipc/src/liteipc_adapter.c"
        )


add_library(
        liteipc_adapter
        SHARED
        ${SOURCES}
)

target_link_directories(liteipc_adapter PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        liteipc_adapter
        libsec_shared
#        log
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)