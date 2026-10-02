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

#include "PipewireProfiler.h"
#include "PipewireManager1.h"
#include "PipewireNode.h"


#include <stdexcept>
#include <spa/pod/parser.h>

namespace UkuiAudioFramwork
{
#define PIPEWIRE_CAST(x) PipewireProfiler* _this = static_cast<PipewireProfiler*>(x);

void profiler_profile(void *data, const struct spa_pod *pod)
{
    PIPEWIRE_CAST(data);
    _this->_profiler_profile(pod);
}

void PipewireProfiler::_profiler_profile(const struct spa_pod *pod)
{
    struct spa_pod *o;
    struct spa_pod_prop *p;
    point point;

    uint32_t pod_size = SPA_POD_BODY_SIZE(pod);
    for (o = static_cast<spa_pod *>(SPA_POD_BODY(pod));
         spa_pod_is_inside(pod, pod_size, o);
         o = (struct spa_pod *)spa_pod_next(o))
    {
        int res = 0;
        if (!spa_pod_is_object_type(o, SPA_TYPE_OBJECT_Profiler))
            continue;

        spa_zero(point);
        SPA_POD_OBJECT_FOREACH((struct spa_pod_object*)o, p)
        {
            switch(p->key) {
            case SPA_PROFILER_info:
                    res = process_info(&p->value, &point.info);
                    break;
            case SPA_PROFILER_clock:
                    res = process_clock(&p->value, &point.info);
                    break;
            case SPA_PROFILER_driverBlock:
                    res = process_driver_block(&p->value, &point);
                    break;
            case SPA_PROFILER_followerBlock:
                    process_follower_block(&p->value, &point);
                    break;
            default:
                    break;
            }
            if (res < 0)
                break;
        }
        if (res < 0)
            continue;
    }
}

static const struct pw_profiler_events profiler_events = {
        .version = PW_VERSION_PROFILER_EVENTS,
        .profile = profiler_profile,
};

// ----------------------------------------------------------------------------

int PipewireProfiler::process_info(const struct spa_pod *pod, struct PipewireNode::driver *info)
{
    return spa_pod_parse_struct(pod,
                    SPA_POD_Long(&info->count),
                    SPA_POD_Float(&info->cpu_load[0]),
                    SPA_POD_Float(&info->cpu_load[1]),
                    SPA_POD_Float(&info->cpu_load[2]),
                    SPA_POD_Int(&info->xrun_count));
}

int PipewireProfiler::process_clock(const struct spa_pod *pod, struct PipewireNode::driver *info)
{
    return spa_pod_parse_struct(pod,
                    SPA_POD_Int(&info->clock.flags),
                    SPA_POD_Int(&info->clock.id),
                    SPA_POD_Stringn(info->clock.name, sizeof(info->clock.name)),
                    SPA_POD_Long(&info->clock.nsec),
                    SPA_POD_Fraction(&info->clock.rate),
                    SPA_POD_Long(&info->clock.position),
                    SPA_POD_Long(&info->clock.duration),
                    SPA_POD_Long(&info->clock.delay),
                    SPA_POD_Double(&info->clock.rate_diff),
                    SPA_POD_Long(&info->clock.next_nsec));
}

int PipewireProfiler::process_driver_block(const struct spa_pod *pod, struct point *point)
{
    char *name = NULL;
    uint32_t id = 0;
    struct PipewireNode::measurement measure;
    PipewireNode *node = nullptr;

    spa_zero(measure);
    int res = spa_pod_parse_struct(pod,
                        SPA_POD_Int(&id),
                        SPA_POD_String(&name),
                        SPA_POD_Long(&measure.prev_signal),
                        SPA_POD_Long(&measure.signal),
                        SPA_POD_Long(&measure.awake),
                        SPA_POD_Long(&measure.finish),
                        SPA_POD_Int(&measure.status),
                        SPA_POD_Fraction(&measure.latency));
    if (res < 0)
        return res;

    {
//        PipewireNode *i;
        for(const auto& it : pipewire->m_nodes) {
            if (it->id() == id) {
                node = it;
                break;
            }
        }
    }

    if (node == nullptr)
        return -ENOENT;

    node->setDriver(node);
    node->setMeasurement(measure);
    node->setInfo(point->info);
    point->driver = node;

    if (measure.status != 3) {
        node->errors++;
//        emit node->errorChanged();
        if (node->last_error_status == -1)
            node->last_error_status = measure.status;
    }

    return 0;
}

int PipewireProfiler::process_follower_block(const struct spa_pod *pod, struct point *point)
{
    uint32_t id = 0;
    const char *name = nullptr;
    struct PipewireNode::measurement measure;
    PipewireNode *node = nullptr;

    spa_zero(measure);
    int res = spa_pod_parse_struct(pod,
                        SPA_POD_Int(&id),
                        SPA_POD_String(&name),
                        SPA_POD_Long(&measure.prev_signal),
                        SPA_POD_Long(&measure.signal),
                        SPA_POD_Long(&measure.awake),
                        SPA_POD_Long(&measure.finish),
                        SPA_POD_Int(&measure.status),
                        SPA_POD_Fraction(&measure.latency));
    if (res < 0)
        return res;

    {
//        PipewireNode *i;
        for(const auto& it : pipewire->m_nodes) {
            if (it->id() == id) {
                node = it;
                break;
            }
        }
    }

    if (node == nullptr)
        return -ENOENT;

    node->setMeasurement(measure);
    node->setDriver(point->driver);
    if (measure.status != 3) {
        node->errors++;
//        emit node->errorChanged();
        if (node->last_error_status == -1)
            node->last_error_status = measure.status;
    }

    return 0;
}


// ----------------------------------------------------------------------------
PipewireProfiler::PipewireProfiler(Pipewire *parent, uint32_t id, const spa_dict* props)
    : /*QObject(parent)
    ,*/ pipewire(parent)
{
    profiler = static_cast<pw_proxy*>(
                   pw_registry_bind(pipewire->registry, id, PW_TYPE_INTERFACE_Profiler, PW_VERSION_PROFILER, 0));
    if (profiler == nullptr) {
        throw std::runtime_error("Error creating profiler proxy");
    }

    pw_proxy_add_object_listener(profiler,
                                 &profiler_listener,
                                 &profiler_events,
                                 this);

    pipewire->resync();
}

PipewireProfiler::~PipewireProfiler()
{
    if (profiler != nullptr) {
        spa_hook_remove(&profiler_listener);
        pw_proxy_destroy(profiler);
    }
}

}
