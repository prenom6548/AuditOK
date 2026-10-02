add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "include"
        "$dsoftbus_root_path/core/common/include"
        "$softbus_adapter_common/include"
        "$dsoftbus_root_path/interfaces/kits/common"
        "$dsoftbus_root_path/interfaces/kits/discovery"
        "$dsoftbus_root_path/sdk/discovery/manager/include"
        "$dsoftbus_root_path/sdk/frame/common/include"
        ${common_log_inc}

        "include"
        "$dsoftbus_root_path/interfaces/kits/discovery"
        "$dsoftbus_root_path/interfaces/kits/common"
        "${dsoftbus_root_path}/sdk/discovery/manager/include"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/sdk/frame/common/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/adapter/common/include"
)

set(SOURCES
        "src/client_disc_service.c"
        )


add_library(
        dsoftbus_disc_service_sdk
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_disc_service_sdk PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_disc_service_sdk
        softbus_adapter
        softbus_log
        dsoftbus_disc_manager_sdk
        softbus_client_frame
        libhilog
)