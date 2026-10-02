add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_core_path}/common/include"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/sdk/transmission/session/include"
        ${common_log_inc}

        "include"
        "${dsoftbus_root_path}/adapter/common/include"
        "${dsoftbus_root_path}/sdk/transmission/ipc/include"
)

set(SOURCES
        "src/client_trans_proxy_manager.c"
        )


add_library(
        dsoftbus_trans_proxy_channel_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_proxy_channel_sdk PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_proxy_channel_sdk
        softbus_log
        dsoftbus_trans_ipc_proxy_sdk
        libhilog
)