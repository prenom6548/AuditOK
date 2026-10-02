/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "../libkymedia.h"
#include <stdio.h>

/**
 * @brief 打印设备信息
 * @param device 设备信息
 */
static void print_device_info(const kdk_device_info_t* device) {
    if (!device) {
        printf("设备信息为空\n");
        return;
    }
    printf("设备名称: %s\n", device->name);
    printf("设备ID: %s\n", device->id);
    printf("子设备: %d/%d\n", device->subdevice + 1, device->subdevices_count);
    printf("子设备名称: %s\n", device->subdevice_name);
    printf("\n");
}

/**
 * @brief 打印设备列表
 * @param list 设备列表
 */
static void print_device_list(const kdk_device_list_t* list) {
    if (!list) {
        printf("设备列表为空\n");
        return;
    }
    printf("设备总数: %d\n", list->count);
    for (int i = 0; i < list->count; i++) {
        printf("设备 %d:\n", i + 1);
        print_device_info(&list->devices[i]);
    }
}

/**
 * @brief 打印声卡信息
 * @param card 声卡信息
 */
static void print_sound_card(const kdk_sound_card_t* card) {
    if (!card) {
        printf("声卡信息为空\n");
        return;
    }
    printf("声卡 %d: %s\n", card->card, card->name);
    printf("声卡描述: %s\n", card->longname);
    printf("设备数量: %d\n", card->device_count);
    for (int i = 0; i < card->device_count; i++) {
        printf("设备 %d:\n", i + 1);
        print_device_info(&card->devices[i]);
    }
    printf("\n");
}

int main() {
    printf("=== 默认输入设备 ===\n");
    kdk_device_info_t* default_input = kdk_media_get_default_input_device();
    if (default_input) {
        print_device_info(default_input);
        kdk_media_free_device_info(default_input);
    } else {
        printf("获取默认输入设备失败\n\n");
    }

    printf("=== 默认输出设备 ===\n");
    kdk_device_info_t* default_output = kdk_media_get_default_output_device();
    if (default_output) {
        print_device_info(default_output);
        kdk_media_free_device_info(default_output);
    } else {
        printf("获取默认输出设备失败\n\n");
    }

    printf("=== 所有输入设备 ===\n");
    kdk_device_list_t* input_devices = kdk_media_get_all_input_device();
    if (input_devices) {
        print_device_list(input_devices);
        kdk_media_free_device_list(input_devices);
    } else {
        printf("获取输入设备列表失败\n\n");
    }

    printf("=== 所有输出设备 ===\n");
    kdk_device_list_t* output_devices = kdk_media_get_all_output_device();
    if (output_devices) {
        print_device_list(output_devices);
        kdk_media_free_device_list(output_devices);
    } else {
        printf("获取输出设备列表失败\n\n");
    }

    printf("=== 所有声卡信息 ===\n");
    kdk_sound_card_t* sound_cards = kdk_media_get_all_sound_card();
    if (sound_cards) {
        int card_count = 0;
        while (card_count < 32 && sound_cards[card_count].card >= 0) {
            print_sound_card(&sound_cards[card_count]);
            card_count++;
        }
        kdk_media_free_sound_card(sound_cards);
    } else {
        printf("获取声卡列表失败\n\n");
    }

    return 0;
}
