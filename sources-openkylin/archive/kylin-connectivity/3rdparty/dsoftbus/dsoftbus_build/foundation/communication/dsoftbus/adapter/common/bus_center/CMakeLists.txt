add_definitions()

include_directories(
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/common/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
        "${DSOFTBUS_ROOT}/foundation/communication/wifi/interfaces/innerkits/native_cpp/wifi_standard/interfaces"
        "${DSOFTBUS_ROOT}/foundation/communication/wifi/services/wifi_standard/wifi_framework/common/net_helper"
        "include"

        "${dsoftbus_root_path}/core/bus_center/utils/include"
        "${dsoftbus_root_path}/interfaces/kits/bus_center"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${DSOFTBUS_ROOT}/depend/hilog/include"
)

set(SOURCES
        "network/lnn_lwip_monitor_virtual.c"
        "network/lnn_netlink_monitor.c"
        "platform/lnn_product_monitor_virtual.c"
        "wlan/lnn_wifiservice_monitor_virtual.cpp"
        )

add_library(
        dsoftbus_bus_center_monitor
        SHARED
        ${SOURCES}
)

#find_library (
##        log-lib
#        log
#)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_libraries(
        dsoftbus_bus_center_monitor
        softbus_adapter
        dsoftbus_bus_center_utils
        softbus_log
        message_handler
        utils
        cesfwk_innerkits
        libhilog
        ipc_core
#        ${log-lib}
)