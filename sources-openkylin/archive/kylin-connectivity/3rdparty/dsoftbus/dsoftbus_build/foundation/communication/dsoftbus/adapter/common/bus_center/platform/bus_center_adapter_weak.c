/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <securec.h>
#include <fcntl.h>
#include <unistd.h>

#include "bus_center_adapter.h"
#include "softbus_adapter_log.h"
#include "softbus_errcode.h"

#define DEFAULT_DEVICE_NAME "UNKNOWN"
#define DEFAULT_UDID_NAME "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00"
#define DEFAULR_DEVICE_NAME_LENGTH 128
#define UUID_BUF_LEN 128
#define UUID_PATH "/data/data/dsoftbus/uuid"

static int32_t GetUuid(char *uuid)
{
    int32_t fd;

    if ((fd = open(UUID_PATH, O_RDWR)) > 0) {
        memset(uuid, 0, UUID_BUF_LEN);

        if (read(fd, uuid, UUID_BUF_LEN) < 0) {
            return close(fd);
        }
        close(fd);
    } else {
        HILOG_ERROR(SOFTBUS_HILOG_ID, "fail: open uuid file error!");
        return SOFTBUS_ERR;
    }

    return SOFTBUS_OK;
}

int __attribute__((weak)) GetCommonDevInfo(const CommonDeviceKey key, char *value, uint32_t len)
{
    if (value == NULL) {
        HILOG_ERROR(SOFTBUS_HILOG_ID, "fail: para error!");
        return SOFTBUS_INVALID_PARAM;
    }
    switch (key) {
    case COMM_DEVICE_KEY_DEVNAME: {
        char host[DEFAULR_DEVICE_NAME_LENGTH];
        gethostname(host, DEFAULR_DEVICE_NAME_LENGTH - 1);
        if (strncpy_s(value, len, host, strlen(host)) != EOK) {
            return SOFTBUS_ERR;
        }
        HILOG_INFO(SOFTBUS_HILOG_ID, "HOSTNAME = %s", value);
        break;
    }

    case COMM_DEVICE_KEY_UDID: {
        char uuid[UUID_BUF_LEN];
        if (GetUuid(uuid) != SOFTBUS_OK) {
            return SOFTBUS_ERR;
        } else {
            if (strncpy_s(value, len, uuid, strlen(uuid)) != EOK) {
                return SOFTBUS_ERR;
            }
        }
        HILOG_INFO(SOFTBUS_HILOG_ID, "UUID = %s", value);
        break;
    }

    case COMM_DEVICE_KEY_DEVTYPE:
        break;
    default:
        break;
    }
    return SOFTBUS_OK;
}
