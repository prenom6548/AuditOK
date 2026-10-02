add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/common/message_handler/include"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/common/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"

        "include"
        "${dsoftbus_root_path}/interfaces/kits"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"

        ${common_log_inc}

        "${dsoftbus_root_path}/adapter/default_config/spec_config"

)

set(SOURCES
        "src/lnn_async_callback_utils.c"
        "src/lnn_connection_addr_utils.c"
        "src/lnn_file_utils.c"
        "src/lnn_ip_utils.c"
        )


add_library(
        dsoftbus_bus_center_utils
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_bus_center_utils PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_bus_center_utils
        softbus_adapter
        message_handler
        softbus_property
        utils
        libhilog
)