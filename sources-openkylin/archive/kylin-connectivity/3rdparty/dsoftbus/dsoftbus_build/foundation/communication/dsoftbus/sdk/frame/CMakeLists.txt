add_definitions("-DSOFTBUS_WATCH")

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "common/include"
        "standard/include"
        "${dsoftbus_root_path}/core/common/include"
        "${softbus_adapter_common}/include"
        "${DSOFTBUS_ROOT}/utils/system/safwk/native/include"
        "${dsoftbus_root_path}/sdk/discovery/manager/include"
        "${dsoftbus_root_path}/core/frame/standard/server/include"
        "${dsoftbus_root_path}/sdk/transmission/session/include"
        "${dsoftbus_root_path}/sdk/bus_center/manager/include"
        "${dsoftbus_root_path}/core/frame/standard/client/include"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${softbus_adapter_config}/spec_config"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        ${common_log_inc}

        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/sdk/transmission/trans_channel/manager/include"
)

set(SOURCES
        "${dsoftbus_root_path}/core/frame/standard/client/src/softbus_client_stub.cpp"
        "common/src/softbus_client_event_manager.c"
        "common/src/softbus_client_frame_manager.c"
        "standard/src/softbus_client_death_recipient.cpp"
        "standard/src/softbus_server_proxy_frame.cpp"
        "standard/src/softbus_server_proxy_standard.cpp"
        )


add_library(
        softbus_client_frame
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(softbus_client_frame PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        softbus_client_frame
        libhilog
        ipc_single
        softbus_log
        softbus_property
        softbus_utils
        dsoftbus_bus_center_manager_sdk
        dsoftbus_disc_manager_sdk
        dsoftbus_trans_session_manager_sdk
        utils

)