add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_root_path}/core/common/include"
        "${DSOFTBUS_ROOT}/third_party/cJSON"
        "${dsoftbus_root_path}/sdk/transmission/trans_channel/tcp_direct"
        "${softbus_adapter_common}/include"

        "include"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/interfaces/kits/common"
        
        ${common_log_inc}

)

set(SOURCES
        "src/softbus_message_open_channel.c"
        )


add_library(
        dsoftbus_trans_common
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_common PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_common
        libhilog
        softbus_adapter
        json_utils
        softbus_log
)