add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_core_path}/common/include"
        "${dsoftbus_root_path}/sdk/transmission/session/include"
        "${dsoftbus_root_path}/sdk/transmission/trans_channel/udp/stream/include"
        "${softbus_adapter_common}/include"
        "file/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/transmission/common/include"
        ${common_log_inc}

        "common/include"
        "${dsoftbus_root_path}/interfaces/kits/common"
)

set(SOURCES
        "common/src/client_trans_udp_virtual.c"
        "file/src/client_trans_file_virtual.c"
        )


add_library(
        dsoftbus_trans_udp_channel_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_trans_udp_channel_sdk PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_trans_udp_channel_sdk
        libhilog
)