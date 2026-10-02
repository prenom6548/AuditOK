#add_definitions("-D_GNU_SOURCE")

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/connection/common/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/connection/manager"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"

        ${common_log_inc}
)

set(SOURCES
        "src/softbus_tcp_connect_manager.c"
        )

add_library(
        tcp_connection
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(tcp_connection PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        tcp_connection
        softbus_adapter
        softbus_log
        softbus_property
        softbus_utils
        conn_common
        libhilog
)
