add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "include"
        "${dsoftbus_root_path}/core/common/include"
        "${softbus_adapter_common}/include"

        ${common_log_inc}

)

set(SOURCES
        "src/trans_pending_pkt.c"
        )


add_library(
        softbus_trans_pending
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(softbus_trans_pending PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        softbus_trans_pending
        libhilog
        softbus_adapter
        softbus_log
        softbus_utils
)