#add_definitions("-D_GNU_SOURCE")

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "include"
        "src"
        "${dsoftbus_root_path}/adapter/common/include"
        "${dsoftbus_root_path}/adapter/common/net/bluetooth/include"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/connection/manager"
        "${dsoftbus_root_path}/core/connection/ble/include"
        "${dsoftbus_root_path}/core/common/message_handler/include"
        "${DSOFTBUS_ROOT}/third_party/cJSON"

        ${common_log_inc}
)

set(SOURCES
        "src/softbus_ble_connection_virtual.c"
        )

add_library(
        ble_connection
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(ble_connection PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        ble_connection
        libhilog
)
