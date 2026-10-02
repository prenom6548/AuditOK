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

#include "libkymedia.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>

/* 定义最大设备数量 */
#define MAX_DEVICE_COUNT 32

/**
 * @brief 获取声卡信息
 * @param card 声卡编号
 * @return 声卡信息，如果失败返回NULL
 */
static kdk_sound_card_t* get_card_info(int card) {
    snd_ctl_t* handle;
    kdk_sound_card_t* card_info = malloc(sizeof(kdk_sound_card_t));
    if (!card_info) return NULL;
    memset(card_info, 0, sizeof(kdk_sound_card_t));

    char name[32];
    snprintf(name, sizeof(name), "hw:%d", card);
    
    if (snd_ctl_open(&handle, name, 0) < 0) {
        free(card_info);
        return NULL;
    }

    snd_ctl_card_info_t* alsa_card_info;
    snd_ctl_card_info_alloca(&alsa_card_info);
    
    if (snd_ctl_card_info(handle, alsa_card_info) < 0) {
        snd_ctl_close(handle);
        free(card_info);
        return NULL;
    }

    card_info->card = card;
    strncpy(card_info->name, snd_ctl_card_info_get_name(alsa_card_info), sizeof(card_info->name) - 1);
    strncpy(card_info->longname, snd_ctl_card_info_get_longname(alsa_card_info), sizeof(card_info->longname) - 1);

    /* 获取设备列表 */
    card_info->devices = malloc(sizeof(kdk_device_info_t) * MAX_DEVICE_COUNT);
    if (!card_info->devices) {
        snd_ctl_close(handle);
        free(card_info);
        return NULL;
    }
    memset(card_info->devices, 0, sizeof(kdk_device_info_t) * MAX_DEVICE_COUNT);

    int device = -1;
    while (snd_ctl_pcm_next_device(handle, &device) >= 0 && device >= 0 && card_info->device_count < MAX_DEVICE_COUNT) {
        snd_pcm_info_t* pcm_info;
        snd_pcm_info_alloca(&pcm_info);
        
        snd_pcm_info_set_device(pcm_info, device);
        snd_pcm_info_set_subdevice(pcm_info, 0);
        snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_PLAYBACK);

        if (snd_ctl_pcm_info(handle, pcm_info) >= 0) {
            kdk_device_info_t* dev = &card_info->devices[card_info->device_count];
            strncpy(dev->name, snd_pcm_info_get_name(pcm_info), sizeof(dev->name) - 1);
            strncpy(dev->id, snd_pcm_info_get_id(pcm_info), sizeof(dev->id) - 1);
            dev->subdevice = snd_pcm_info_get_subdevice(pcm_info);
            dev->subdevices_count = snd_pcm_info_get_subdevices_count(pcm_info);
            strncpy(dev->subdevice_name, snd_pcm_info_get_subdevice_name(pcm_info), sizeof(dev->subdevice_name) - 1);
            card_info->device_count++;
        }

        /* 获取输入设备信息 */
        snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_CAPTURE);
        if (snd_ctl_pcm_info(handle, pcm_info) >= 0) {
            kdk_device_info_t* dev = &card_info->devices[card_info->device_count];
            strncpy(dev->name, snd_pcm_info_get_name(pcm_info), sizeof(dev->name) - 1);
            strncpy(dev->id, snd_pcm_info_get_id(pcm_info), sizeof(dev->id) - 1);
            dev->subdevice = snd_pcm_info_get_subdevice(pcm_info);
            dev->subdevices_count = snd_pcm_info_get_subdevices_count(pcm_info);
            strncpy(dev->subdevice_name, snd_pcm_info_get_subdevice_name(pcm_info), sizeof(dev->subdevice_name) - 1);
            card_info->device_count++;
        }
    }

    snd_ctl_close(handle);
    return card_info;
}

/**
 * @brief 获取默认输入设备
 * @return 默认输入设备信息，如果失败返回NULL
 */
kdk_device_info_t* kdk_media_get_default_input_device(void) {
    kdk_device_list_t* list = kdk_media_get_all_input_device();
    if (!list || list->count == 0) {
        if (list) kdk_media_free_device_list(list);
        return NULL;
    }

    // 假设第一个设备是默认设备（这是简化处理）
    kdk_device_info_t* default_device = malloc(sizeof(kdk_device_info_t));
    if (!default_device) {
        kdk_media_free_device_list(list);
        return NULL;
    }
    memcpy(default_device, &list->devices[0], sizeof(kdk_device_info_t));
    
    kdk_media_free_device_list(list);
    return default_device;
}

/**
 * @brief 获取默认输出设备
 * @return 默认输出设备信息，如果失败返回NULL
 */
kdk_device_info_t* kdk_media_get_default_output_device(void) {
    kdk_device_list_t* list = kdk_media_get_all_output_device();
    if (!list || list->count == 0) {
        if (list) kdk_media_free_device_list(list);
        return NULL;
    }

    // 假设第一个设备是默认设备（这是简化处理）
    kdk_device_info_t* default_device = malloc(sizeof(kdk_device_info_t));
    if (!default_device) {
        kdk_media_free_device_list(list);
        return NULL;
    }
    memcpy(default_device, &list->devices[0], sizeof(kdk_device_info_t));
    
    kdk_media_free_device_list(list);
    return default_device;
}

/**
 * @brief 获取所有输入设备列表
 * @return 输入设备列表，如果失败返回NULL
 */
kdk_device_list_t* kdk_media_get_all_input_device(void) {
    kdk_device_list_t* list = malloc(sizeof(kdk_device_list_t));
    if (!list) return NULL;
    memset(list, 0, sizeof(kdk_device_list_t));

    list->devices = malloc(sizeof(kdk_device_info_t) * MAX_DEVICE_COUNT);
    if (!list->devices) {
        free(list);
        return NULL;
    }
    memset(list->devices, 0, sizeof(kdk_device_info_t) * MAX_DEVICE_COUNT);

    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0 && list->count < MAX_DEVICE_COUNT) {
        snd_ctl_t* handle;
        char name[32];
        snprintf(name, sizeof(name), "hw:%d", card);
        
        if (snd_ctl_open(&handle, name, 0) >= 0) {
            int device = -1;
            while (snd_ctl_pcm_next_device(handle, &device) >= 0 && device >= 0 && list->count < MAX_DEVICE_COUNT) {
                snd_pcm_info_t* pcm_info;
                snd_pcm_info_alloca(&pcm_info);
                
                snd_pcm_info_set_device(pcm_info, device);
                snd_pcm_info_set_subdevice(pcm_info, 0);
                snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_CAPTURE);

                if (snd_ctl_pcm_info(handle, pcm_info) >= 0) {
                    kdk_device_info_t* dev = &list->devices[list->count];
                    strncpy(dev->name, snd_pcm_info_get_name(pcm_info), sizeof(dev->name) - 1);
                    strncpy(dev->id, snd_pcm_info_get_id(pcm_info), sizeof(dev->id) - 1);
                    dev->subdevice = snd_pcm_info_get_subdevice(pcm_info);
                    dev->subdevices_count = snd_pcm_info_get_subdevices_count(pcm_info);
                    strncpy(dev->subdevice_name, snd_pcm_info_get_subdevice_name(pcm_info), sizeof(dev->subdevice_name) - 1);
                    list->count++;
                }
            }
            snd_ctl_close(handle);
        }
    }

    return list;
}

/**
 * @brief 获取所有输出设备列表
 * @return 输出设备列表，如果失败返回NULL
 */
kdk_device_list_t* kdk_media_get_all_output_device(void) {
    kdk_device_list_t* list = malloc(sizeof(kdk_device_list_t));
    if (!list) return NULL;
    memset(list, 0, sizeof(kdk_device_list_t));

    list->devices = malloc(sizeof(kdk_device_info_t) * MAX_DEVICE_COUNT);
    if (!list->devices) {
        free(list);
        return NULL;
    }
    memset(list->devices, 0, sizeof(kdk_device_info_t) * MAX_DEVICE_COUNT);

    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0 && list->count < MAX_DEVICE_COUNT) {
        snd_ctl_t* handle;
        char name[32];
        snprintf(name, sizeof(name), "hw:%d", card);
        
        if (snd_ctl_open(&handle, name, 0) >= 0) {
            int device = -1;
            while (snd_ctl_pcm_next_device(handle, &device) >= 0 && device >= 0 && list->count < MAX_DEVICE_COUNT) {
                snd_pcm_info_t* pcm_info;
                snd_pcm_info_alloca(&pcm_info);
                
                snd_pcm_info_set_device(pcm_info, device);
                snd_pcm_info_set_subdevice(pcm_info, 0);
                snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_PLAYBACK);

                if (snd_ctl_pcm_info(handle, pcm_info) >= 0) {
                    kdk_device_info_t* dev = &list->devices[list->count];
                    strncpy(dev->name, snd_pcm_info_get_name(pcm_info), sizeof(dev->name) - 1);
                    strncpy(dev->id, snd_pcm_info_get_id(pcm_info), sizeof(dev->id) - 1);
                    dev->subdevice = snd_pcm_info_get_subdevice(pcm_info);
                    dev->subdevices_count = snd_pcm_info_get_subdevices_count(pcm_info);
                    strncpy(dev->subdevice_name, snd_pcm_info_get_subdevice_name(pcm_info), sizeof(dev->subdevice_name) - 1);
                    list->count++;
                }
            }
            snd_ctl_close(handle);
        }
    }

    return list;
}

/**
 * @brief 设置默认输入设备
 * @param devicename 要设置的设备名称
 * @return 成功返回0，失败返回-1
 */
int kdk_media_set_default_input_device(const char* devicename) {
    if (!devicename) return -1;
    
    snd_pcm_t* handle;
    if (snd_pcm_open(&handle, devicename, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        return -1;
    }
    
    snd_pcm_close(handle);
    return 0;
}

/**
 * @brief 设置默认输出设备
 * @param devicename 要设置的设备名称
 * @return 成功返回0，失败返回-1
 */
int kdk_media_set_default_output_device(const char* devicename) {
    if (!devicename) return -1;
    
    snd_pcm_t* handle;
    if (snd_pcm_open(&handle, devicename, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        return -1;
    }
    
    snd_pcm_close(handle);
    return 0;
}

/**
 * @brief 获取所有声卡信息
 * @return 声卡列表，如果失败返回NULL
 */
kdk_sound_card_t* kdk_media_get_all_sound_card(void) {
    kdk_sound_card_t* cards = malloc(sizeof(kdk_sound_card_t) * MAX_DEVICE_COUNT);
    if (!cards) return NULL;
    memset(cards, 0, sizeof(kdk_sound_card_t) * MAX_DEVICE_COUNT);

    int card = -1;
    int card_count = 0;

    while (snd_card_next(&card) >= 0 && card >= 0 && card_count < MAX_DEVICE_COUNT) {
        kdk_sound_card_t* card_info = get_card_info(card);
        if (card_info) {
            memcpy(&cards[card_count], card_info, sizeof(kdk_sound_card_t));
            card_count++;
            free(card_info);
        }
    }

    /* 标记数组结束 */
    if (card_count < MAX_DEVICE_COUNT) {
        cards[card_count].card = -1;
    }

    return cards;
}

/**
 * @brief 释放设备信息内存
 * @param device 要释放的设备信息
 */
void kdk_media_free_device_info(kdk_device_info_t* device) {
    if (device) {
        free(device);
    }
}

/**
 * @brief 释放设备列表内存
 * @param list 要释放的设备列表
 */
void kdk_media_free_device_list(kdk_device_list_t* list) {
    if (list) {
        if (list->devices) {
            free(list->devices);
        }
        free(list);
    }
}

/**
 * @brief 释放声卡信息内存
 * @param card 要释放的声卡信息
 */
void kdk_media_free_sound_card(kdk_sound_card_t* card) {
    if (card) {
        if (card->devices) {
            free(card->devices);
        }
        free(card);
    }
}
