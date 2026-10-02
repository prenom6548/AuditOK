add_definitions("-DSOFTBUS_STANDARD_SYSTEM -DDEFAULT_STORAGE_PATH=\\\"/data/data\\\"")

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_core_path}/common/include"
        "include"
        "${DSOFTBUS_ROOT}/third_party/cJSON"
        "${softbus_adapter_config}/spec_config"
        "${DSOFTBUS_ROOT}/utils/native/base/include"
)

set(SOURCES
        "src/softbus_feature_config.c"
        )


add_library(
        softbus_property
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(softbus_property PUBLIC /opt/kylin-connectivity/dsoftbus/)
target_link_libraries(
        softbus_property
        softbus_adapter
        cjson_static
        utils
        libhilog
)