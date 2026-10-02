add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_core_path}/common/include"
        "${dsoftbus_sdk_path}/frame/common/include"
        ${common_log_inc}

        "common/include"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/sdk/transmission/trans_channel/manager/include"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${dsoftbus_root_path}/adapter/default_config/spec_config"
        "${dsoftbus_root_path}/sdk/transmission/trans_channel/udp/file/include"
        "${dsoftbus_root_path}/interfaces/inner_kits/transport"
        "${DSOFTBUS_ROOT}/third_party/cJSON"
        "${dsoftbus_root_path}/sdk/transmission/ipc/include"
        "${dsoftbus_root_path}/sdk/transmission/session/cpp/include"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
)

set(SOURCES
        "${dsoftbus_root_path}/sdk/transmission/session/cpp/src/session_callback_mock.cpp"
        "${dsoftbus_root_path}/sdk/transmission/session/cpp/src/session_impl.cpp"
        "${dsoftbus_root_path}/sdk/transmission/session/cpp/src/session_mock.cpp"
        "${dsoftbus_root_path}/sdk/transmission/session/cpp/src/session_service_impl.cpp"

        "src/client_trans_message_service.c"
        "src/client_trans_session_service.c"
        )


add_library(
        dsoftbus_trans_session_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_session_sdk PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_session_sdk
        json_utils
        softbus_log
        softbus_property
        softbus_utils
        dsoftbus_bus_center_service_sdk
        softbus_client_frame
        dsoftbus_trans_session_manager_sdk
        dsoftbus_trans_ipc_proxy_sdk
        utils
        libhilog
)