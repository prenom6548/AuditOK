set(dsoftbus_root_path "${DSOFTBUS_ROOT}/foundation/communication/dsoftbus")
set(dsoftbus_sdk_path "${DSOFTBUS_ROOT}/foundation/communication/dsoftbus/sdk")
set(dsoftbus_core_path "${DSOFTBUS_ROOT}/foundation/communication/dsoftbus/core")
set(hispark_pegasus_sdk_path "${DSOFTBUS_ROOT}/device/hisilicon/hispark_pegasus/sdk_liteos")
set(softbus_adapter_common "${DSOFTBUS_ROOT}/foundation/communication/dsoftbus/adapter/common")
set(softbus_adapter_config "${DSOFTBUS_ROOT}/foundation/communication/dsoftbus/adapter/default_config")


set(common_log_inc
        "${DSOFTBUS_ROOT}/depend/hilog/include"
        "${DSOFTBUS_ROOT}/third_party/bounds_checking_function/include"
        )

add_subdirectory(third_party)
add_subdirectory(depend)
add_subdirectory(utils)

add_subdirectory(foundation)