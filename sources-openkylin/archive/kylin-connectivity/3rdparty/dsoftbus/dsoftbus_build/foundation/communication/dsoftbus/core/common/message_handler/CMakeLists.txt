add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_root_path}/core/common/include"
        "${softbus_adapter_common}/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"

        ${common_log_inc}
        "include"
)

set(SOURCES
        "message_handler.c"
        )


add_library(
        message_handler
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(message_handler PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        message_handler
        softbus_adapter
        softbus_log
        utils
        libhilog
)