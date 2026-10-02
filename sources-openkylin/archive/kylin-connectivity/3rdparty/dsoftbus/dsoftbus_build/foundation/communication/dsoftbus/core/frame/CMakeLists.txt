add_subdirectory(standard)
add_subdirectory(small)

add_definitions()

set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "common/include"
        "small/init/include"
        "small/client_manager/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/bus_center/utils/include"
        "${dsoftbus_root_path}/core/common/message_handler/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/common/inner_communication"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/connection/manager"
        "${dsoftbus_root_path}/core/transmission/trans_channel/proxy/include"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${DSOFTBUS_ROOT}/base/security/deviceauth/interfaces/innerkits"
        "${DSOFTBUS_ROOT}/utils/native/lite/include"
        "${softbus_adapter_config}/spec_config"

        ${common_log_inc}

        "${dsoftbus_root_path}/core/authentication/include"
        "${DSOFTBUS_ROOT}/foundation/communication/ipc_lite/interfaces/kits"
        "${dsoftbus_root_path}/core/authentication/interface"
        "${dsoftbus_root_path}/core/bus_center/ipc/include"
        "${dsoftbus_root_path}/core/discovery/interface"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/core/discovery/manager/include"
        "${DSOFTBUS_ROOT}/depend/device_auth/include"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/core/transmission/trans_channel/manager/include"
        "${dsoftbus_root_path}/core/transmission/session/include"
        "${dsoftbus_root_path}/core/transmission/trans_channel/auth/include"
        "${DSOFTBUS_ROOT}/foundation/communication/ipc_lite/interfaces/kits"
)

set(SOURCES
        "common/src/softbus_server_frame.c"
        "small/init/src/bus_center_server_stub.c"
        "small/init/src/disc_server_stub.c"
        "small/init/src/softbus_server_stub.c"
        "small/init/src/trans_server_stub.c"
        )


add_library(
        softbus_server_frame
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)

target_link_directories(softbus_server_frame PUBLIC /opt/connectivity/dsoftbus/)

target_link_libraries(
        softbus_server_frame
        softbus_adapter
        dsoftbus_auth_server
        dsoftbus_bus_center_server
        softbus_log
        message_handler
        softbus_permission
        softbus_utils
        conn_manager
        dsoftbus_disc_server
        client_manager
        dsoftbus_trans_session
        liteipc_adapter
#        samgr
)