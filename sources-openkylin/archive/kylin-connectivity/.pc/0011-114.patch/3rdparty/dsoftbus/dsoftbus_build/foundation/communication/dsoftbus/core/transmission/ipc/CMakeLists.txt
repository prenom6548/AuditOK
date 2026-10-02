add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "standard/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/frame/standard/client/include"
        "${dsoftbus_root_path}/core/frame/standard/softbusdata/include"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/interfaces/kits/common"

        "include"
        "${dsoftbus_root_path}/core/connection/interface"

        ${common_log_inc}

        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
        "${dsoftbus_root_path}/adapter/common/include"

)

set(SOURCES
        "standard/src/trans_client_proxy.cpp"
        "standard/src/trans_client_proxy_standard.cpp"
        )


add_library(
        dsoftbus_trans_ipc_proxy
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_ipc_proxy PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_ipc_proxy
        libhilog
        softbus_log
        softbus_server_data
        utils
)