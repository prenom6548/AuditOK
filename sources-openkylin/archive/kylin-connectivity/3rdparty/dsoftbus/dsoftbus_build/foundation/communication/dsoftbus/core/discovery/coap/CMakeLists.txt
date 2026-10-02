#add_definitions("-D_GNU_SOURCE")

#set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_C_FLAGS "-Wno-error")

include_directories(
        "include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/bus_center/interface"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/core/discovery/interface"
        "${dsoftbus_root_path}/core/discovery/manager/include"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${dsoftbus_root_path}/interfaces/kits/discovery"
        "${DSOFTBUS_ROOT}/third_party/cJSON"

        ${common_log_inc}

        "${dsoftbus_root_path}/components/nstackx/nstackx_ctrl/interface"
)

set(SOURCES
        "src/disc_coap.c"
        "src/disc_nstackx_adapter.c"
        )

add_library(
        dsoftbus_disc_coap
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(dsoftbus_disc_coap PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        dsoftbus_disc_coap
        softbus_adapter
        nstackx_ctrl
        dsoftbus_bus_center_ledger
        json_utils
        softbus_log
        libhilog
)
