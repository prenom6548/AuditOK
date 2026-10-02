add_definitions()

#set(CMAKE_CXX_FLAGS "-Wall")

include_directories(
        "${dsoftbus_core_path}/common/include"
        "${dsoftbus_core_path}/common/security/permission/include"
        "${DSOFTBUS_ROOT}/foundation/appexecfwk/standard/interfaces/innerkits/appexecfwk_core/include/bundlemgr"
        "${DSOFTBUS_ROOT}/foundation/appexecfwk/standard/kits/appkit/native/app/include"
        "${DSOFTBUS_ROOT}/foundation/appexecfwk/standard/interfaces/innerkits/appexecfwk_base/include"
        "${DSOFTBUS_ROOT}/foundation/aafwk/standard/interfaces/innerkits/want/include"
        "${DSOFTBUS_ROOT}/utils/system/safwk/native/include"

        "${dsoftbus_root_path}/adapter/common/include"
        "${DSOFTBUS_ROOT}/third_party/cJSON"

        ${common_log_inc}
        "${DSOFTBUS_ROOT}/foundation/communication/ipc_lite/interfaces/kits"
)

set(SOURCES
        "common/permission_entry.c"
        "standard_system/permission_utils.cpp"
        "standard_system/softbus_permission.cpp"
        )


add_library(
        softbus_permission
        SHARED
        ${SOURCES}
)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output/lib/dsoftbus)
target_link_directories(softbus_permission PUBLIC /opt/connectivity/dsoftbus/)
target_link_libraries(
        softbus_permission
        json_utils
        softbus_utils
        softbus_log
        cjson_static
        utils
        want
        appexecfwk_core
        libhilog
        ipc_core
        libpermissionsdk_standard
        samgr_proxy
)