/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DATAMANAGEMENT_COMMON_H
#define DATAMANAGEMENT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { UNKNOWN = -1, AVAILABLE = 0, NOT_INSTALLED } FeatureStatus;

typedef enum {
    SUCCESS = 0,
    PARSE_PARAM_FAILED = 14,
    DM_ERROR_UNKNOWN = 100,
    DM_SESSION_ERROR = 101,
} ErrorCode;

typedef struct {
    bool success;        // 表示操作是否成功
    int errorCode;       // 错误代码，0表示无错误
    char errorMsg[500];  // 错误消息
} DataManagementStatus;

#ifdef __cplusplus
}
#endif

#endif  // DATAMANAGEMENT_COMMON_H
