/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "stdio.h"
#include "../libkyusb.h"

#define BUS_NUM 10
#define DEV_NUM 50

int main()
{
    pDriverInfoList list = kdk_usb_get_list();
    if(list)
    {
        pDriverInfoList tmp = list;
        while (tmp)
        {
            printf("Name: %s\t",tmp->data->name);
            printf("Class: %s\t",tmp->data->type);
            printf("PID: %s\t",tmp->data->pid);
            printf("VID: %s\t",tmp->data->vid);
            printf("Serial: %s\t",tmp->data->serialNo);
            printf("DevNode: %s\n",tmp->data->devNode);
            tmp = tmp->next;
        }
        
    }
    kdk_usb_free(list);

    pUsbInfo usb_info = kdk_usb_get_bus_info();
    pUsbInfo tmp = usb_info;
    while(tmp)
    {
        printf("%03u:%03u\n", tmp->busNum, tmp->devNum);
        tmp = tmp->next;
    }
    printf("%d\n", kdk_usb_get_mount(BUS_NUM,DEV_NUM));
    char *product_name = kdk_usb_get_productName(BUS_NUM,DEV_NUM);
    printf("product_name %s\n", product_name);
    char *manufacturer_name = kdk_usb_get_manufacturerName(BUS_NUM,DEV_NUM);
    printf("manufacturer_name %s\n", manufacturer_name);
    char *version = kdk_usb_get_version(BUS_NUM,DEV_NUM);
    printf("version %s\n", version);
    printf("%d\n", kdk_usb_get_deviceClass(BUS_NUM,DEV_NUM));
    printf("%d\n", kdk_usb_get_deviceSubClass(BUS_NUM,DEV_NUM));
    printf("%d\n", kdk_usb_get_deviceProtocol(BUS_NUM,DEV_NUM));
    char *speed = kdk_usb_get_speed(BUS_NUM,DEV_NUM);
    printf("speed %s\n", speed);
    pUsbDeviceDescriptor desc = kdk_usb_get_device_descriptor(BUS_NUM,DEV_NUM);
    printf("%d %d %d %d\n",desc->lenth,desc->descriptorType, desc->maxPacketSize0, desc->numConfigurations);
    return 0;
}
