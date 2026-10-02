add_definitions()

#set(CMAKE_C_FLAGS "-Wall -fPIC -std=c99")

include_directories(
        "include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/sdk/transmission/trans_channel/manager/include"
        "common/include"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/interfaces/kits"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${dsoftbus_root_path}/interfaces/kits/transport"
        "${dsoftbus_root_path}/sdk/transmission/session/cpp/include"
        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
        "${dsoftbus_root_path}/sdk/bus_center/manager/include"
        "${dsoftbus_root_path}/sdk/discovery/manager/include"
        "${dsoftbus_root_path}/adapter/common/include"
        ${common_log_inc}
)

set(SOURCES
        "src/if_softbus_client.cpp"
        "src/softbus_client_stub.cpp"
        src/discunixdeviceinfo.cpp
        src/tlv.cpp
        )


add_library(
        softbus_client
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)

add_executable(connectivity_softbus_client src/softbus_client_main.cpp)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/bin)

target_link_directories(softbus_client PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_directories(connectivity_softbus_client PUBLIC /opt/kylin-connectivity/dsoftbus/)

target_link_libraries(connectivity_softbus_client PUBLIC softbus_client
        )

target_include_directories(connectivity_softbus_client PUBLIC
        "include"
        )

target_link_libraries(
        softbus_client
        softbus_log
        dsoftbus_bus_center_manager_sdk
        dsoftbus_bus_center_service_sdk
        dsoftbus_disc_manager_sdk
        dsoftbus_disc_service_sdk
        softbus_client_frame
        dsoftbus_trans_session_manager_sdk
        dsoftbus_trans_session_sdk
        utils
        libhilog
        ipc_single
)