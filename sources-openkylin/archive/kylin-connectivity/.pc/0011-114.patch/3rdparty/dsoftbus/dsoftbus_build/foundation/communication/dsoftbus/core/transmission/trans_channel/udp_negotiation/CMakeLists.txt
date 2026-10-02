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
        "${dsoftbus_root_path}/core/bus_center/lnn/lane_hub/lane_manager/include"
        "${dsoftbus_root_path}/core/transmission/common"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/interfaces/kits/common"

        ${common_log_inc}
)

set(SOURCES
        "src/trans_udp_virtual.c"
        )


add_library(
        dsoftbus_trans_udp_channel
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_udp_channel PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_udp_channel
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