add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_root_path}/core/common/softbus_property/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
        "include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/common/include"
        "${softbus_adapter_config}/spec_config"

        ${common_log_inc}
)

set(SOURCES
        "softbus_log.c"
        )


add_library(
        softbus_log
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(softbus_log PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        softbus_log
        softbus_adapter
        softbus_property
        utils
        libhilog
)