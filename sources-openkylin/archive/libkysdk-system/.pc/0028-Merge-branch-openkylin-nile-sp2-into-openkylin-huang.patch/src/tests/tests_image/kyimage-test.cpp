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

#include <stdio.h>
#include <time.h>
#include "../../image/libkyimageproc.h"
#include <gtest/gtest.h>

double my_get_timestamp()
{
  struct timespec a;
  clock_gettime(CLOCK_MONOTONIC, &a);
  return ((double)a.tv_nsec * 1.0e-9) + (double)a.tv_sec;
}

TEST(KdkImageTest, ImageInfo)
{
    double start = my_get_timestamp();
    void * handle = kdk_jpeg_encoder_init();
    double end = my_get_timestamp();
    printf("jpeg encoder init %.2fms\n", (end - start) * 1000);

    char *image = "/home/kylin/sdk/libkysdk-system/libkysdk-system/src/tests/tests_image/testorig.ppm";

    start = my_get_timestamp();
    kdk_jpeg_encode_to_file(handle, image,"dest.jpg");
    end = my_get_timestamp();
   	printf("encode image %.2fms\n", (end - start) * 1000);
    kdk_jpeg_encoder_release(handle);

    kdk_image_init init;
    init.size.width = 300;
    init.size.height = 500;

    handle = kdk_image_create_pixelmap(init);

    handle = kdk_image_load_pixelmap("/home/kylin/test.png");
    kdk_image_info *info = kdk_image_get_image_info(handle);
    {
      printf("width = %d, height = %d\n", info->size.width, info->size.height);
      kdk_free_image_info(info);
    }

    int bytes = kdk_image_get_bytes_number_per_row(handle);
    printf("bytes = %d\n", bytes);

    int total = kdk_image_get_bytes_number_total(handle);
    printf("total = %d\n", total);

    kdk_image_set_opacity(handle, 0.5);
    kdk_image_set_scale(handle, 600, 300);
    kdk_image_set_rotate(handle, 50);
    kdk_image_set_flip(handle, true, true);
    kdk_image_set_crop(handle, 10, 10, 300, 150);
    int ret = kdk_image_save(handle, "/home/kylin/szm.png");
    printf("ret = %d\n", ret);
}