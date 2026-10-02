add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "include"
        "${dsoftbus_root_path}/core/common/include"
        "${dsoftbus_root_path}/adapter/common/include"

        ${common_log_inc}
)

set(SOURCES
        "src/softbus_sequence_verification.c"
        )


add_library(
        softbus_seq_verification
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(softbus_seq_verification PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        softbus_seq_verification
        libhilog
        softbus_log
)