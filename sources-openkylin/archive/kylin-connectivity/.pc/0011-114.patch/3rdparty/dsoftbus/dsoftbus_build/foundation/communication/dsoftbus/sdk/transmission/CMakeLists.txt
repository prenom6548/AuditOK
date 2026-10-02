add_subdirectory(ipc)
add_subdirectory(session)
add_subdirectory(trans_channel)

add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_core_path}/common/include"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_sdk_path}/bus_center/manager/include"
        "${dsoftbus_sdk_path}/transmission/trans_channel/auth/include"
        ${common_log_inc}

        "session/include"
        "trans_channel/manager/include"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/sdk/transmission/trans_channel/proxy/include"
        "${dsoftbus_root_path}/sdk/transmission/trans_channel/tcp_direct/include"
        "${dsoftbus_root_path}/core/common/security/sequence_verification/include"
        "${dsoftbus_root_path}/sdk/transmission/trans_channel/udp/common/include"
        "${dsoftbus_root_path}/sdk/transmission/ipc/include"
)

set(SOURCES
        "session/src/client_trans_session_callback.c"
        "session/src/client_trans_session_manager.c"
        "trans_channel/manager/src/client_trans_channel_callback.c"
        "trans_channel/manager/src/client_trans_channel_manager.c"
        )


add_library(
        dsoftbus_trans_session_manager_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_session_manager_sdk PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_session_manager_sdk
        softbus_adapter
        softbus_utils
        dsoftbus_bus_center_manager_sdk
        dsoftbus_trans_ipc_proxy_sdk
        dsoftbus_trans_auth_channel_sdk
        dsoftbus_trans_proxy_channel_sdk
        dsoftbus_trans_direct_channel_sdk
        dsoftbus_trans_udp_channel_sdk
        utils
        libhilog
)