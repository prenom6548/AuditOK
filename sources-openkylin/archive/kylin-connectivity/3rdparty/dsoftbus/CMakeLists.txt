cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_CXX_STANDARD 17)
project("dsoftbus")

#set(CMAKE_OBJECT_PATH_MAX 500)
set(DSOFTBUS_ROOT ${CMAKE_SOURCE_DIR}/3rdparty/dsoftbus/dsoftbus_build)
set(dsoftbus_root_path "${DSOFTBUS_ROOT}/foundation/communication/dsoftbus")
add_subdirectory(dsoftbus_build)

add_executable(connectivity_softbus_server server.cpp)
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/bin)

target_include_directories (
        connectivity_softbus_server
        PRIVATE
        ${common_inc_dir}
        includes
        "${dsoftbus_root_path}/core/frame/common/include"
)

target_link_directories(connectivity_softbus_server PUBLIC /opt/connectivity/dsoftbus/)

target_link_libraries (
        connectivity_softbus_server

        softbus_server
        softbus_adapter
        dsoftbus_bus_center_monitor
        dsoftbus_auth_server
        dsoftbus_bus_center_ledger
        dsoftbus_bus_center_hub
        dsoftbus_bus_center_utils
        dsoftbus_bus_center_server
        softbus_log
        message_handler
        softbus_property
        softbus_utils
        json_utils
        softbus_permission
        conn_common
        conn_manager
        ble_connection
        br_connection
        tcp_connection
        dsoftbus_disc_server
        ble_discovery
        dsoftbus_disc_coap
        softbus_server_data
        dsoftbus_trans_session
        dsoftbus_trans_ipc_proxy
        dsoftbus_trans_proxy_channel
        dsoftbus_trans_udp_channel
        dsoftbus_trans_direct_channel
        softbus_trans_pending
        dsoftbus_trans_common
        nstackx_ctrl
        nstackx_util.open
        libcoap
        libhilog
        ipc_core
        deviceauth_sdk
        pthread
        rt
)
