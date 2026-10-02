add_subdirectory(ble)
add_subdirectory(coap)

include_directories(
        "ble/include"
        "coap/include"
        "ipc/include"
        "ipc/standard/include"
        "${dsoftbus_root_path}/core/common/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/frame/standard/client/include"
        "${dsoftbus_root_path}/core/frame/standard/softbusdata/include"
        "${DSOFTBUS_ROOT}/utils/system/safwk/native/include"

        ${common_log_inc}

        "service/include"
        "interface"
        "manager/include"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/connection/interface"
        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
)

set(SOURCES
        "ipc/standard/src/disc_client_proxy.cpp"
        "ipc/standard/src/disc_client_proxy_standard.cpp"
        "manager/src/disc_manager.c"
        "manager/src/softbus_disc_server.c"
        )

add_library(
        dsoftbus_disc_server
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_disc_server PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_disc_server
        libhilog
        ipc_core
        softbus_adapter
        softbus_log
        softbus_utils
        softbus_server_data
        ble_discovery
        dsoftbus_disc_coap
        utils
)
