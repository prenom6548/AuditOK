add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/common/security/include"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/transmission/ipc/include"
        "${dsoftbus_root_path}/core/transmission/trans_channel/auth/include"
        "${dsoftbus_root_path}/core/bus_center/utils/include"
        "${dsoftbus_root_path}/core/bus_center/lnn/lane_hub/lane_manager/include"
        "${dsoftbus_root_path}/core/bus_center/lnn/net_builder/include"
        "{DSOFTBUS_ROOT}/third_party/cJSON"

        ${common_log_inc}

        "include"
        "${dsoftbus_root_path}/core/authentication/interface"
        "${dsoftbus_root_path}/core/transmission/trans_channel/manager/include"
        "${dsoftbus_root_path}/core/transmission/trans_channel/auth/include"
        "${dsoftbus_root_path}/interfaces/kits/common"

        "${DSOFTBUS_ROOT}/third_party/cJSON"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/core/transmission/trans_channel/proxy/include"
        "${dsoftbus_root_path}/core/transmission/trans_channel/tcp_direct/include"
        "${dsoftbus_root_path}/core/transmission/trans_channel/udp_negotiation/include"
)

set(SOURCES
        "${dsoftbus_root_path}/core/transmission/trans_channel/auth/src/trans_auth_manager.c"
        "${dsoftbus_root_path}/core/transmission/trans_channel/auth/src/trans_auth_message.c"
        "${dsoftbus_root_path}/core/transmission/trans_channel/manager/src/trans_channel_callback.c"
        "${dsoftbus_root_path}/core/transmission/trans_channel/manager/src/trans_channel_manager.c"
        "${dsoftbus_root_path}/core/transmission/trans_channel/manager/src/trans_lane_manager.c"
        "src/trans_session_manager.c"
        "src/trans_session_service.c"
        )


add_library(
        dsoftbus_trans_session
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_session PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_session
        libhilog
        softbus_permission
        dsoftbus_trans_ipc_proxy
        softbus_adapter
        dsoftbus_auth_server
        dsoftbus_bus_center_server
        dsoftbus_bus_center_hub
        dsoftbus_bus_center_ledger
        dsoftbus_bus_center_utils
        json_utils
        softbus_log
        softbus_utils
        dsoftbus_trans_ipc_proxy
        dsoftbus_trans_proxy_channel
        dsoftbus_trans_direct_channel
        dsoftbus_trans_udp_channel
        utils
)