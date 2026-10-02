/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#pragma once

#include <spa/utils/hook.h>

#include <pipewire/proxy.h>
#include <pipewire/extensions/profiler.h>

#include "PipewireNode.h"

namespace UkuiAudioFramwork
{
class Pipewire;

class PipewireProfiler
{

private:
    Pipewire *pipewire = nullptr;
    pw_proxy *profiler = nullptr;
    spa_hook profiler_listener;
    int check_profiler;

    struct point {
        struct PipewireNode* driver;
        struct PipewireNode::driver info;
    };

public:
    explicit PipewireProfiler(Pipewire *parent, uint32_t id, const spa_dict* props);
    virtual ~PipewireProfiler();

    int process_info(const struct spa_pod *pod, struct PipewireNode::driver *info);
    int process_clock(const struct spa_pod *pod, struct PipewireNode::driver *info);
    int process_driver_block(const struct spa_pod *pod, struct point *point);
    int process_follower_block(const struct spa_pod *pod, struct point *point);

    // private
    void _profiler_profile(const struct spa_pod *pod);

};
}
