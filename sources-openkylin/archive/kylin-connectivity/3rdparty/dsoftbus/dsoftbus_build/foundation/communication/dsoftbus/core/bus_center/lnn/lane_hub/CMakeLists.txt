add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "lane_manager/include"
        "time_sync/include"
        "${dsoftbus_root_path}/core/bus_center/lnn/net_ledger/common/include"
        "${dsoftbus_root_path}/core/bus_center/service/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/common/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"

        ${common_log_inc}

        "include"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/interfaces/kits"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
)

set(SOURCES
        "lane_manager/src/lnn_lane_info.c"
        "lane_manager/src/lnn_lane_manager.c"
        "lane_manager/src/lnn_smart_communication.c"
        "time_sync/src/lnn_time_sync_manager_stub.c"
        )


add_library(
        dsoftbus_bus_center_hub
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_bus_center_hub PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_bus_center_hub
        softbus_adapter
        dsoftbus_bus_center_ledger
        softbus_log
        utils
        libhilog
)