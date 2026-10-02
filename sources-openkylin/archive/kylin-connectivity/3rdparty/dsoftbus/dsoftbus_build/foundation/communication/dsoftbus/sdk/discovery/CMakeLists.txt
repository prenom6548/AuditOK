add_subdirectory(service)

add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "ipc/include"
        "ipc/standard/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/frame/standard/server/include"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${DSOFTBUS_ROOT}/utils/system/safwk/native/include"
        "${softbus_adapter_common}/include"
        ${common_log_inc}

        "manager/include"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/connection/interface"
        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
)

set(SOURCES
        "${dsoftbus_root_path}/sdk/discovery/ipc/standard/src/disc_server_proxy.cpp"
        "${dsoftbus_root_path}/sdk/discovery/ipc/standard/src/disc_server_proxy_standard.cpp"
        "manager/src/client_disc_manager.c"
        )


add_library(
        dsoftbus_disc_manager_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_disc_manager_sdk PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_disc_manager_sdk
        libhilog
        ipc_single
        softbus_adapter
        softbus_log
        utils
)