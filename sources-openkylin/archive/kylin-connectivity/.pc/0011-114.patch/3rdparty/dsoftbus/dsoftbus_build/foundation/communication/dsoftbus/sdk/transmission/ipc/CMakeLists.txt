add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "standard/include"
        "${DSOFTBUS_ROOT}/utils/system/safwk/native/include"
        "${dsoftbus_root_path}/core/frame/standard/server/include"
        "${dsoftbus_root_path}/core/transmission/common/include"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/interfaces/kits/common"
        ${common_log_inc}
        "include"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/connection/interface"

        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
        "${dsoftbus_root_path}/adapter/common/include"
)

set(SOURCES
        "standard/src/trans_server_proxy.cpp"
        "standard/src/trans_server_proxy_standard.cpp"
        )


add_library(
        dsoftbus_trans_ipc_proxy_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_ipc_proxy_sdk PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_ipc_proxy_sdk
        libhilog
        ipc_single
        softbus_log
        utils
)