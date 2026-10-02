add_definitions("-D_GNU_SOURCE")

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/connection/common/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${softbus_adapter_common}/include"

        ${common_log_inc}
)

set(SOURCES
        "src/softbus_base_listener.c"
        "src/softbus_tcp_socket.c"
        "src/softbus_thread_pool.c"
        )

add_library(
        conn_common
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(conn_common PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        conn_common
        softbus_adapter
        softbus_log
        utils
        libhilog
)
