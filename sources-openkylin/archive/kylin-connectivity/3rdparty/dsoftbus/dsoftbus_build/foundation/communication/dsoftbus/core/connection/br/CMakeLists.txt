#add_definitions("-D_GNU_SOURCE")

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "include"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/connection/br/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/adapter/br/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/common/message_handler/include"
        "${dsoftbus_root_path}/core/connection/manager"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"
        "${DSOFTBUS_ROOT}/third_party/cJSON"

        ${common_log_inc}
)

set(SOURCES
        "src/br_connection_virtual.c"
        )

add_library(
        br_connection
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(br_connection PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        br_connection
        libhilog
)
