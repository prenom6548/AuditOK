include_directories(
        "include"
        "$dsoftbus_root_path/core/common/include"

        ${common_log_inc}

        "${DSOFTBUS_ROOT}/depend/ipc/include"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/adapter/common/include"
)

set(SOURCES
        "src/softbus_server_data.cpp"
        )

add_library(
        softbus_server_data
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)

target_link_directories(softbus_server_data PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        softbus_server_data
        libhilog
        ipc_core
        softbus_log
        utils
)
