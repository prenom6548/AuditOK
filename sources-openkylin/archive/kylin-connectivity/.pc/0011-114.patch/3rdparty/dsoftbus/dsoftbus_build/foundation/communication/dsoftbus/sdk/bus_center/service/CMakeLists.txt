add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "include"
        "${dsoftbus_root_path}/core/common/include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/interfaces/kits"
        "${dsoftbus_root_path}/sdk/bus_center/manager/include"
        "${dsoftbus_root_path}/sdk/frame/common/include"

        ${common_log_inc}

        "manager/include"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/interfaces/kits"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"
        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
)

set(SOURCES
        "src/client_bus_center.c"
        )


add_library(
        dsoftbus_bus_center_service_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)

target_link_directories(dsoftbus_bus_center_service_sdk PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_bus_center_service_sdk
        softbus_adapter
        softbus_utils
        dsoftbus_bus_center_manager_sdk
        softbus_client_frame
        libhilog
)