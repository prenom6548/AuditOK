add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_root_path}/interfaces/kits"
        "${dsoftbus_root_path}/interfaces/kits/common"
        "${DSOFTBUS_ROOT}/third_party/mbedtls/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"

        "${softbus_adapter_common}/include"
        "${softbus_adapter_config}/spec_config"
        "${dsoftbus_root_path}/core/common/include"

        ${common_log_inc}
)

set(SOURCES
        "${softbus_adapter_common}/bus_center/platform/bus_center_adapter_weak.c"
        "${softbus_adapter_common}/kernel/liteos_a/softbus_adapter_file.c"
        "${softbus_adapter_common}/kernel/liteos_a/softbus_adapter_mem.c"
        "${softbus_adapter_common}/kernel/liteos_a/softbus_adapter_timer.c"
        "${softbus_adapter_common}/log/softbus_adapter_log.c"
        "${softbus_adapter_common}/mbedtls/softbus_adapter_crypto.c"
        "${softbus_adapter_config}/spec_config/softbus_config_adapter.c"
        )

add_library(
        softbus_adapter
        SHARED
        ${SOURCES}
)

#find_library (
##        log-lib
#        log
#)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)

target_link_directories(softbus_adapter PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        softbus_adapter
        mbedtls_shared
        utils
        libsec_shared
        libhilog
        pthread
        rt
#        ${log-lib}
)

add_subdirectory(common)