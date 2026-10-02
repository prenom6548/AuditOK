add_definitions()

set(CMAKE_C_FLAGS "-Wall -fPIC")

include_directories(
        "include"
        "${softbus_adapter_common}/include"
        "${dsoftbus_root_path}/core/common/include"
        "${DSOFTBUS_ROOT}/third_party/bounds_checking_function/include"

        ${common_log_inc}
)

set(SOURCES
        "src/softbus_client_info_manager.c"
        )


add_library(
        client_manager
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_libraries(
        client_manager
        softbus_adapter
        softbus_log
        softbus_utils
        libsec_shared
)