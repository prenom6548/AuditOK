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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sensors/sensors.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <langinfo.h>
#include "libkyfan.h"

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif


static char *fan_print_label(const char *label, int space)
{
    char tmp[20] = "\0";
    char *ret = (char *)malloc(sizeof(char) * 20);
    // printf("%s:%*s", label, space - len, "");
    snprintf(tmp, sizeof(tmp), "%s:", label);
    strncpy(ret, tmp, 19);
    return ret;
}

static int fan_get_input_value(const sensors_chip_name *name, const sensors_subfeature *sub, double *val)
{
    int err;

    err = sensors_get_value(name, sub->number, val);
    return err;
}

static char* print_chip_fan(const sensors_chip_name *name, const sensors_feature *feature, int label_size)
{
    const sensors_subfeature *sfub;
    double val;
    char *label;
    char tmp[64] = "\0";
    
    if (!(label = sensors_get_label(name, feature)))
    {
        return NULL;
    }
    char *ret = fan_print_label(label, label_size);
    free(label);

    sfub = sensors_get_subfeature(name, feature, SENSORS_SUBFEATURE_FAN_FAULT);
    sfub = sensors_get_subfeature(name, feature, SENSORS_SUBFEATURE_FAN_INPUT);
    if (sfub && fan_get_input_value(name, sfub, &val) == 0)
    {
        snprintf(tmp, sizeof(tmp), "%s%4.0f RPM", ret, val);
        free(ret);
        char *fan = (char *)malloc(strnlen(tmp, STRNLEN_MAX) + 1);
        if(!fan)
        {
            return NULL;
        }
        strncpy(fan, tmp, strnlen(tmp, STRNLEN_MAX) + 1);
        return fan;
    }
    else
    {
        free(ret);
        return NULL;
    }
}

static int fan_get_label_size(const sensors_chip_name *name)
{
    int i;
    const sensors_feature *iter;
    char *label;
    unsigned int max_size = 11; /* 11 as minimum label width */

    i = 0;
    while ((iter = sensors_get_features(name, &i)))
    {
        if ((label = sensors_get_label(name, iter)) && strnlen(label, STRNLEN_MAX) > max_size)
            max_size = strnlen(label, STRNLEN_MAX);
        free(label);
    }

    /* One more for the colon, and one more to guarantee at least one
	   space between that colon and the value */
    return max_size + 2;
}

static char** fan_print_chip(const sensors_chip_name *name)
{
    const sensors_feature *feature;
    int label_size;
    char *ret = NULL;
    char **fan = NULL;

    label_size = fan_get_label_size(name);

    int j = 0;
    int nr = 0;
    int count = 0;
    while ((feature = sensors_get_features(name, &nr)))
        count++;

    nr = 0;
    while ((feature = sensors_get_features(name, &nr)))
    {
        switch (feature->type)
        {
        case SENSORS_FEATURE_FAN:
        {
            fan = calloc(count + 1, sizeof(char *));
            if (NULL == fan)
                goto out;

            ret = print_chip_fan(name, feature, label_size);
            if(!ret)
            {
                continue;
            }
            fan[j++] = ret;
        }
            break;
        default:
            continue;
        }
    }
out:
    return fan;
}

char** kdk_fan_get_information()
{
    int err;
    const char *config_file_name = NULL;
    FILE *fan_config_file;

    setlocale(LC_CTYPE, "");

    if (config_file_name)
    {
        if (!strcmp(config_file_name, "-"))
            fan_config_file = stdin;
        else
            fan_config_file = fopen(config_file_name, "r");

        if (!fan_config_file)
        {
            perror(config_file_name);
            return NULL;
        }
    }
    else
    {
        fan_config_file = NULL;
    }

    err = sensors_init(fan_config_file);
    if (err)
    {
        if (fan_config_file)
            fclose(fan_config_file);
        return NULL;
    }

    if (fan_config_file && fclose(fan_config_file) == EOF)
        perror(config_file_name);

    const sensors_chip_name *fan_match = NULL;
    const sensors_chip_name *fan_chip;
    int index = 0;
    char **fan = NULL;

    int chip_nr = 0;
    while ((fan_chip = sensors_get_detected_chips(fan_match, &chip_nr)))
    {

        fan = fan_print_chip(fan_chip);
    }
    sensors_cleanup();
    return fan;
err_out:
    while (fan[index])
    {
        free(fan[index]);
        index ++;
    }
    free(fan);
    return NULL;
}

inline void kdk_fan_freeall(char **list)
{
    if (! list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index ++;
    }
    free(list);
}
