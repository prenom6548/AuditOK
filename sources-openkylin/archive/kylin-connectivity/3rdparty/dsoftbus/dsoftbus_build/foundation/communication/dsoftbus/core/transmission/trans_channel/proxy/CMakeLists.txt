add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "include"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/interfaces/kits/common"

        "${dsoftbus_root_path}/core/transmission/trans_channel/manager/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/transmission/interface"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/connection/manager"
        "${dsoftbus_root_path}/core/authentication/interface"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${softbus_adapter_common}/include"
        "${DSOFTBUS_ROOT}/third_party/cJSON"

        ${common_log_inc}

        "${dsoftbus_root_path}/core/transmission/pending_packet/include"
        "${dsoftbus_root_path}/core/common/message_handler/include"

)

set(SOURCES
        "src/softbus_proxychannel_callback.c"
        "src/softbus_proxychannel_control.c"
        "src/softbus_proxychannel_listener.c"
        "src/softbus_proxychannel_manager.c"
        "src/softbus_proxychannel_message.c"
        "src/softbus_proxychannel_network.c"
        "src/softbus_proxychannel_session.c"
        "src/softbus_proxychannel_transceiver.c"
        )


add_library(
        dsoftbus_trans_proxy_channel
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_proxy_channel PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_proxy_channel
        json_utils
        message_handler
        dsoftbus_trans_common
        dsoftbus_trans_ipc_proxy
        softbus_adapter
        softbus_trans_pending
        softbus_utils
        dsoftbus_auth_server
        dsoftbus_bus_center_ledger
        message_handler
        conn_manager
        libhilog
)