add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "common/include"
        "distributed_ledger/include"
        "local_ledger/include"
        "sync_ledger/include"
        "utils/include"
        "${dsoftbus_root_path}/core/adapter/bus_center/include"
        "${dsoftbus_root_path}/core/common/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"

        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/interfaces/kits"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"

        ${common_log_inc}
)

set(SOURCES
        "common/src/lnn_device_info.c"
        "common/src/lnn_map.c"
        "common/src/lnn_net_capability.c"
        "common/src/lnn_node_info.c"
        "distributed_ledger/src/lnn_distributed_net_ledger.c"
        "local_ledger/src/lnn_local_net_ledger.c"
        "net_ledger.c"
        )


add_library(
        dsoftbus_bus_center_ledger
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_bus_center_ledger PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_bus_center_ledger
        softbus_adapter
        json_utils
        softbus_property
        softbus_utils
        utils
        libhilog
)