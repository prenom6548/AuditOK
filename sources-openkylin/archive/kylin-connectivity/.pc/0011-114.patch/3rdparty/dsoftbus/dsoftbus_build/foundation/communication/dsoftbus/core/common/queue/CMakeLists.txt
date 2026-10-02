add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_root_path}/adapter/common/include"
        "${dsoftbus_root_path}/core/common/include"
        "${DSOFTBUS_ROOT}/third_party/cJSON"

        ${common_log_inc}
)

set(SOURCES
        "softbus_queue.c"
        )


add_library(
        softbus_queue
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(softbus_queue PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        softbus_queue
        softbus_adapter
        softbus_log
        utils
        libhilog
)