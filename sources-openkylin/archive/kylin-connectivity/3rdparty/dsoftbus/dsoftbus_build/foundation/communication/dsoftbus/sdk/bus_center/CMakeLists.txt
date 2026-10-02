add_subdirectory(service)

add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "ipc/include"
        "ipc/standard/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/core/frame/standard/server/include"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"
        "${DSOFTBUS_ROOT}/utils/system/safwk/native/include"
        ${common_log_inc}

        "manager/include"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/interfaces/kits"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"
        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
)

set(SOURCES
        "ipc/standard/src/bus_center_server_proxy.cpp"
        "ipc/standard/src/bus_center_server_proxy_standard.cpp"
        "manager/src/client_bus_center_manager.c"
        )


add_library(
        dsoftbus_bus_center_manager_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_bus_center_manager_sdk PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_bus_center_manager_sdk
        softbus_adapter
        softbus_property
        softbus_utils
        utils
        libhilog
        ipc_single
)