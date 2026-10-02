#add_definitions("-D_GNU_SOURCE")

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "include"
        "${dsoftbus_root_path}/adapter/common/include"
        "${dsoftbus_root_path}/adapter/common/net/bluetooth/include"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/core/bus_center/lnn/net_ledger/common/include"
        "${dsoftbus_root_path}/core/discovery/interface"
        "${dsoftbus_root_path}/core/discovery/manager/include"
        "${dsoftbus_root_path}/core/discovery/ble/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/connection/manager"
        "${DSOFTBUS_ROOT}/third_party/cJSON"
        
        ${common_log_inc}
)

set(SOURCES
        "src/disc_ble_virtual.c"
        )

add_library(
        ble_discovery
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(ble_discovery PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        ble_discovery
        libhilog
)
