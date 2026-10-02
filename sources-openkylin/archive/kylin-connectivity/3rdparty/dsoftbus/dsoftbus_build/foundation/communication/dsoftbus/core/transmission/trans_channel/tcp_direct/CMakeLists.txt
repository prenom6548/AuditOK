add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${DSOFTBUS_ROOT}/third_party/cJSON"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/common/wifi_common/base_listener/include"
        "${dsoftbus_root_path}/core/common/wifi_common/tcp_socket/include"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/core/authentication/interface"
        "${dsoftbus_root_path}/core/transmission/session/include"

        "include"
        "${dsoftbus_root_path}/core/transmission/trans_channel/manager/include"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/interfaces/kits/common"

        ${common_log_inc}
)

set(SOURCES
        "src/trans_tcp_direct_callback.c"
        "src/trans_tcp_direct_listener.c"
        "src/trans_tcp_direct_manager.c"
        "src/trans_tcp_direct_message.c"
        )


add_library(
        dsoftbus_trans_direct_channel
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_direct_channel PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_direct_channel
        softbus_utils
        conn_common
        dsoftbus_trans_common
        dsoftbus_trans_ipc_proxy
        softbus_adapter
        dsoftbus_auth_server
        dsoftbus_bus_center_ledger
        cjson_static
        libhilog

)