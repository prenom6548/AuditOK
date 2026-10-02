#include <stdio.h>

#include "id_tables.h"
#include "libfidoukey/fidoukey_realization.h"

#include <biometric_common.h>
#include <biometric_stroge.h>
#include <biometric_version.h>


// 该函数名称不可变动
int ops_configure(bio_dev * dev, GKeyFile * conf)
{
    int ret = 0;

    // 设置驱动简称和全称
    dev->device_name = BIO_NAME;
    dev->full_name = BIO_FULL_NAME;

    // 设置驱动所支持的USB设备的ID表
    // 支持的ID表在id_tables.h中
    dev->usb_info.id_table = bio_drv_fido_id_table;
    // 保留字
    dev->usb_info.driver_data = 0;

    dev->ops_configure = ops_configure;

    ret = fidoukey_ops_configure(dev, conf);

    return ret;
}
