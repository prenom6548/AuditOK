#add_definitions("-D_GNU_SOURCE")

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "include"
        "interface"

        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/core/bus_center/lnn/net_ledger/common/include"
        "${dsoftbus_root_path}/core/bus_center/utils/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/common/message_handler/include"
        "${dsoftbus_root_path}/core/connection/manager"
        "${dsoftbus_root_path}/core/connection/interface"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${softbus_adapter_config}/spec_config"
        "${DSOFTBUS_ROOT}/base/security/deviceauth/interfaces/innerkits"
        "${DSOFTBUS_ROOT}/third_party/cJSON"
        "${DSOFTBUS_ROOT}/depend/device_auth/include"

        ${common_log_inc}
)

set(SOURCES
        "src/auth_common.c"
        "src/auth_connection.c"
        "src/auth_manager.c"
        "src/auth_sessionkey.c"
        "src/auth_socket.c"
        )

add_library(
        dsoftbus_auth_server
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_auth_server PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_auth_server
        softbus_adapter
        dsoftbus_bus_center_ledger
        dsoftbus_bus_center_utils
        json_utils
        softbus_log
        message_handler
        softbus_property
        softbus_utils
        conn_common
        conn_manager
        deviceauth_sdk
        libhilog
)
