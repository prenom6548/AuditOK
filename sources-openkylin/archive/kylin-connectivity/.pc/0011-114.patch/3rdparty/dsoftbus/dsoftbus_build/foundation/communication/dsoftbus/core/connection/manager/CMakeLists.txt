#add_definitions("-D_GNU_SOURCE")

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/connection/ble/include"
        "${dsoftbus_root_path}/core/connection/br/include"
        "${dsoftbus_root_path}/core/connection/tcp/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/connection/manager"
        "${dsoftbus_root_path}/interfaces/kits/common"

        ${common_log_inc}
)

set(SOURCES
        "softbus_conn_manager.c"
        )

add_library(
        conn_manager
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(conn_manager PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        conn_manager
        softbus_adapter
        softbus_log
        softbus_utils
        ble_connection
        br_connection
        tcp_connection
        libhilog
)
