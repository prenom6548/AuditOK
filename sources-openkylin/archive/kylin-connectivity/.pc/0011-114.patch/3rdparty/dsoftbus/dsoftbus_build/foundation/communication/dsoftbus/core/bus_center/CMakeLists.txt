add_subdirectory(utils)
add_subdirectory(lnn)

add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "ipc/standard_system/include"
        "lnn/lane_hub/lane_manager/include"
        "lnn/lane_hub/time_sync/include"
        "lnn/net_builder/include"
        "lnn/net_builder/sync_info/include"
        "lnn/net_ledger/common/include"
        "lnn/net_ledger/distributed_ledger/include"
        "lnn/net_ledger/local_ledger/include"
        "${dsoftbus_root_path}/core/adapter/bus_center/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/authentication/interface"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/common/message_handler/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/transmission/interface"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/core/transmission/trans_channel/tcp_direct/include"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/core/discovery/interface"
        "${dsoftbus_root_path}/core/frame/standard/client/include"
        "${dsoftbus_root_path}/core/frame/standard/softbusdata/include"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"
        "${DSOFTBUS_ROOT}/utils/system/safwk/native/include"
        "lnn/net_buscenter/include"
        "monitor/include"

        ${common_log_inc}

        "ipc/include"
        "service/include"
        "interface"
        "${dsoftbus_root_path}/interfaces/kits"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"

        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${dsoftbus_root_path}/core/bus_center/utils/include"
        "${dsoftbus_root_path}/adapter/common/bus_center/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
        "${DSOFTBUS_ROOT}/third_party/cJSON"
)

set(SOURCES
        "ipc/standard_system/src/bus_center_client_proxy.cpp"
        "ipc/standard_system/src/bus_center_client_proxy_standard.cpp"
        "ipc/standard_system/src/lnn_bus_center_ipc.cpp"
        "lnn/net_builder/src/lnn_connection_fsm.c"
        "lnn/net_builder/src/lnn_discovery_manager.c"
        "lnn/net_builder/src/lnn_net_builder.c"
        "lnn/net_builder/src/lnn_network_id.c"
        "lnn/net_builder/src/lnn_node_weight.c"
        "lnn/net_builder/src/lnn_state_machine.c"
        "lnn/net_builder/sync_info/src/lnn_exchange_device_info.c"
        "lnn/net_builder/sync_info/src/lnn_sync_item_info.c"
        "lnn/net_buscenter/src/lnn_ip_network_impl.c"
        "lnn/net_buscenter/src/lnn_network_manager.c"
        "monitor/src/lnn_event_monitor.c"
        "service/src/bus_center_event.c"
        "service/src/bus_center_manager.c"
        "lnn/net_builder/src/lnn_coap_discovery_impl.c"
        )


add_library(
        dsoftbus_bus_center_server
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_bus_center_server PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_bus_center_server
        softbus_adapter
        dsoftbus_bus_center_monitor
        dsoftbus_auth_server
        json_utils
        softbus_log
        message_handler
        softbus_property
        softbus_utils
        conn_manager
        dsoftbus_disc_server
        softbus_server_data
        dsoftbus_trans_proxy_channel
        dsoftbus_trans_direct_channel
        dsoftbus_bus_center_hub
        dsoftbus_bus_center_ledger
        dsoftbus_bus_center_utils
        utils
        libhilog
        ipc_core
)