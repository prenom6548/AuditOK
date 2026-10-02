/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * Author: Droiing <jianglinxuan@kylinos.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMUNITY_MULTIDEVICE_DISCOVER_TOOL_H
#define COMMUNITY_MULTIDEVICE_DISCOVER_TOOL_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <libfprint-2/fprint.h>
#include <community_define.h>

#define PROGAM_NAME "community-multidevice-discover-tool"

typedef struct device_info_t device_info;
struct device_info_t
{
	int drv_id;
	char * drv_name;
	bool exist;
};

typedef struct driver_info_t driver_info;
struct driver_info_t
{
	GPtrArray *devices;
	FpDevice *device;
	FpContext *ctx;
};

#endif // COMMUNITY_MULTIDEVICE_DISCOVER_TOOL_H
