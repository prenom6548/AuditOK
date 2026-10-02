add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_root_path}/core/common/include"
        "${softbus_adapter_common}/include"

        ${common_log_inc}
)

set(SOURCES
        "softbus_utils.c"
        )


add_library(
        softbus_utils
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(softbus_utils PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        softbus_utils
        softbus_adapter
        softbus_log
        utils
        libhilog
)