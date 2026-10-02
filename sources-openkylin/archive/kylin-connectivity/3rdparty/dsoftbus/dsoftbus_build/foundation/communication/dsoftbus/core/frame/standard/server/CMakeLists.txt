add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/core/frame/standard/server/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/frame/common/include"
        "${dsoftbus_root_path}/core/frame/standard/softbusdata/include"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/core/transmission/trans_channel/manager/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/authentication/interface"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"
        ${common_log_inc}
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/core/bus_center/ipc/include"
        "${dsoftbus_root_path}/core/common/message_handler/include"
        "${dsoftbus_root_path}/core/discovery/manager/include"
        "${dsoftbus_root_path}/core/discovery/interface"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/adapter/common/include"
        "${dsoftbus_root_path}/core/transmission/session/include"
        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
)

set(SOURCES
        "${dsoftbus_root_path}/core/frame/standard/server/src/softbus_server_death_recipient.cpp"
        "${dsoftbus_root_path}/core/frame/standard/server/src/softbus_server_frame.c"
        "src/if_softbus_server.cpp"
        "src/softbus_server.cpp"
        "src/softbus_server_stub.cpp"
        )


add_library(
        softbus_server
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)

target_link_directories(softbus_server PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        softbus_server
        softbus_adapter
        dsoftbus_auth_server
        dsoftbus_bus_center_server
        softbus_log
        message_handler
        softbus_property
        softbus_utils
        conn_manager
        dsoftbus_disc_server
        softbus_server_data
        dsoftbus_trans_session
        utils
        libhilog
        ipc_core
        system_ability_fwk
)