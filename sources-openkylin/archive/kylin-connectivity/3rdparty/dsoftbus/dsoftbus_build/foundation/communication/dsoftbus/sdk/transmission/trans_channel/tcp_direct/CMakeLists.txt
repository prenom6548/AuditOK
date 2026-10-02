add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/common/security/sequence_verification/include"
        "${dsoftbus_root_path}/core/common/wifi_common/tcp_socket/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/core/authentication/interface"
        "${dsoftbus_root_path}/sdk/transmission/session/include"
        ${common_log_inc}

        "include"
        "${dsoftbus_core_path}/common/security/sequence_verification/include"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/transmission/pending_packet/include"
        "${dsoftbus_root_path}/adapter/default_config/spec_config"
)

set(SOURCES
        "src/client_trans_tcp_direct_callback.c"
        "src/client_trans_tcp_direct_listener.c"
        "src/client_trans_tcp_direct_manager.c"
        "src/client_trans_tcp_direct_message.c"
        )


add_library(
        dsoftbus_trans_direct_channel_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_direct_channel_sdk PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_direct_channel_sdk
        softbus_seq_verification
        softbus_adapter
        softbus_log
        softbus_property
        softbus_utils
        conn_common
        softbus_trans_pending
)