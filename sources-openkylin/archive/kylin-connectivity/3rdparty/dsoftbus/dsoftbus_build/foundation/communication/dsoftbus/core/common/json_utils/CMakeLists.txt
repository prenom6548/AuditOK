add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_root_path}/core/common/include"
        "${DSOFTBUS_ROOT}/third_party/cJSON"
        "${dsoftbus_root_path}/adapter/common/include"

        ${common_log_inc}
)

set(SOURCES
        "softbus_json_utils.c"
        )


add_library(
        json_utils
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(json_utils PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        json_utils
        softbus_log
        cjson_static
        libsec_shared
        libhilog
)