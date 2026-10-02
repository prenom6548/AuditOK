/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
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

#include <stdio.h>
#include "../libkysearch.h"
#include <stdlib.h>

int main()
{
    char **list = NULL;

    // list = kdk_search_get_term_by_directory("/home/kylin", "test");
    // if (list)
    // {
    //     for (int i = 0; i < 100; i++)
    //     {
    //         if (NULL == list[i])
    //             break;
    //         printf("term : %s\n", list[i]);
    //         free(list[i]);
    //     }
    //     free(list);
    //     list = NULL;
    // }

    // list = kdk_search_full_get_term_by_directory("/home/kylin", "test");
    // if (list)
    // {
    //     for (int i = 0; i < 100; i++)
    //     {
    //         if (NULL == list[i])
    //             break;
    //         printf("term : %s\n", list[i]);
    //         free(list[i]);
    //     }
    //     free(list);
    //     list = NULL;
    // }

    list = kdk_search_get_term_by_filter("/home/kylin", "test", FileType_ALL, FileSize_ALL, FileTime_ALL);
    if (list)
    {
        for (int i = 0; i < 100; i++)
        {
            if (NULL == list[i])
                break;
            printf("filter term : %s\n", list[i]);
            free(list[i]);
        }
        free(list);
        list = NULL;
    }

    KdkImageInfo *info = NULL;

    // info = kdk_search_get_image_by_directory("/home/kylin", "test");
    // if (info)
    // {
    //     printf("image:\n");
    //     KdkImageInfo *tmp = info;
    //     while (tmp)
    //     {
    //         printf("\t path: %s\n", tmp->path);
    //         printf("\t format: %s\n", tmp->format);
    //         printf("\t size: %s\n", tmp->size);
    //         printf("\t colormode: %s\n", tmp->color_mode);
    //         tmp = tmp->next;
    //     }
    //     kdk_search_destory_image_info(info);
    //     info = NULL;
    // }

    // info = kdk_search_full_get_image_by_directory("/home/kylin", "run-tests.svg");
    // if (info)
    // {
    //     printf("full image:\n");
    //     KdkImageInfo *tmp = info;
    //     while (tmp)
    //     {
    //         printf("\t path: %s\n", tmp->path);
    //         printf("\t format: %s\n", tmp->format);
    //         printf("\t size: %s\n", tmp->size);
    //         printf("\t colormode: %s\n", tmp->color_mode);
    //         tmp = tmp->next;
    //     }
    //     kdk_search_destory_image_info(info);
    //     info = NULL;
    // }

    // kdk_search_by_browser("dsfsd");
    // kdk_search_by_application("sett");
}