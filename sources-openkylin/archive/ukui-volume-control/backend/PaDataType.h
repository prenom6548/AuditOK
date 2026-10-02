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

#ifndef PADATATYPE_H
#define PADATATYPE_H

#include <memory>
#include <vector>
#include <list>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <inttypes.h>
#include <iostream>
#include <mutex>
#include <atomic>
#include <condition_variable>

extern "C" {
#include <pulse/introspect.h>
#include <pulse/operation.h>
#include <pulse/context.h>
#include <pulse/cdecl.h>
#include <pulse/gccmacro.h>
#include <pulse/channelmap.h>
#include <pulse/volume.h>
#include <pulse/proplist.h>
#include <pulse/format.h>
#include <pulse/version.h>
}

#include "IStream.h"
#include "Util.h"
#include "DeviceManagerModule.h"

namespace UkuiAudioFramwork
{
#define PA_VOLUME_NORMAL      65536.0
#define UKMEDIA_VOLUME_NORMAL 100.0

struct PaDataInfo;
struct PaSinkPortInfo;
struct PaCardPortInfo;
class IStream;
class PaSinkStream;
class PaSourceStream;
class DeviceManagerModule;

template <typename GetDeviceListFunc>
bool contains(PaDataInfo& data, GetDeviceListFunc getDeviceList, const std::string& name)
{
    auto list = getDeviceList(data);
    for (const auto& p : list) {
        if (p && p->name == name)
            return true;
    }
    return false;
}

typedef struct PaSinkPortInfo {
    std::string name;                   /**< Name of this port */
    std::string description;            /**< Description of this port */
    uint32_t priority;                  /**< The higher this value is, the more useful this port is as a default. */
    int available;                      /**< A flags (see #pa_port_available), indicating availability status of this port. \since 2.0 */
    DevicePortType type = DevicePortType::PORT_TYPE_UNKNOWN;

    PaSinkPortInfo(const std::string& cardName, const pa_sink_port_info& other) {
        name = other.name ? other.name : "";
        description = other.description ? other.description : "";
        priority = other.priority;
        available = other.available;
        type = getDevicePortType(cardName, other.name ? other.name : "");
    }
} PaSinkPortInfo;

typedef struct PaSourcePortInfo {
    std::string name;                   /**< Name of this port */
    std::string description;            /**< Description of this port */
    uint32_t priority;                  /**< The higher this value is, the more useful this port is as a default. */
    int available;                      /**< A flags (see #pa_port_available), indicating availability status of this port. \since 2.0 */
    DevicePortType type;

    PaSourcePortInfo(const std::string& cardName, const pa_source_port_info& other) {
        name = other.name;
        description = other.description;
        priority = other.priority;
        available = other.available;
        type = getDevicePortType(cardName, other.name);
    }
} PaSourcePortInfo;

typedef struct PaModuleInfo {
    uint32_t index;                     /**< Index of the module */
    std::string name;                    /**< Name of the module */
    const char* argument;                      /**< Argument string of the module */
    uint32_t n_used;                    /**< Usage counter or PA_INVALID_INDEX */
/** \cond fulldocs */
    int auto_unload;                    /**< \deprecated Non-zero if this is an autoloaded module. */
/** \endcond */
    pa_proplist *proplist;              /**< Property list \since 0.9.15 */

    void operator =(const pa_module_info& other)
    {
        index = other.index;
        name = other.name ? other.name : "";
        argument = other.argument ? other.argument : "";
        n_used = other.n_used;
        auto_unload = other.auto_unload;
        proplist = other.proplist;
    }

    std::string toString() const;
} PaModuleInfo;

typedef struct PaCardProfileInfo {
    std::string name;                   /**< Name of this profile */
    std::string description;            /**< Description of this profile */
    uint32_t n_sinks;                   /**< Number of sinks this profile would create */
    uint32_t n_sources;                 /**< Number of sources this profile would create */
    uint32_t priority;                  /**< The higher this value is, the more useful this profile is as a default. */

    PaCardProfileInfo(const pa_card_profile_info& other) {
        name = other.name;
        description = other.description;
        n_sinks = other.n_sinks;
        n_sources = other.n_sources;
        priority = other.priority;
    }

} PaCardProfileInfo;

typedef struct PaCardProfileInfo2 {
    std::string name;                   /**< Name of this profile */
    std::string description;            /**< Description of this profile */
    uint32_t n_sinks;                   /**< Number of sinks this profile would create */
    uint32_t n_sources;                 /**< Number of sources this profile would create */
    uint32_t priority;                  /**< The higher this value is, the more useful this profile is as a default. */
    int available;

    PaCardProfileInfo2(const pa_card_profile_info2& other) {
        name = other.name;
        description = other.description;
        n_sinks = other.n_sinks;
        n_sources = other.n_sources;
        priority = other.priority;
        available = other.available;
    }

    void operator =(const pa_card_profile_info2& other)
    {
        name = other.name;
        description = other.description;
        n_sinks = other.n_sinks;
        n_sources = other.n_sources;
        priority = other.priority;
        available = other.available;
    }
} PaCardProfileInfo2;

typedef struct PaCardPortInfo {
    std::string name;                   /**< Name of this port */
    std::string description;            /**< Description of this port */
    uint32_t priority;                  /**< The higher this value is, the more useful this port is as a default. */
    int available;                      /**< A #pa_port_available enum, indicating availability status of this port. */
    int direction;                      /**< A #pa_direction enum, indicating the direction of this port. */
    uint32_t n_profiles;                /**< Number of entries in profile array */
    std::vector<std::shared_ptr<PaCardProfileInfo>> profiles;    /**< \deprecated Superseded by profiles2 */
    pa_proplist *proplist;              /**< Property list */
    int64_t latency_offset;             /**< Latency offset of the port that gets added to the sink/source latency when the port is active. \since 3.0 */
    std::vector<std::shared_ptr<PaCardProfileInfo2>> profiles2;  /**< Array of pointers to available profiles, or NULL. Array is terminated by an entry set to NULL. \since 5.0 */
    bool enabled;
    DevicePortType type;

    PaCardPortInfo(const std::string& cardName, const pa_card_port_info& other, bool enable) {
        name = other.name;
        description = other.description;
        priority = other.priority;
        available = other.available;
        direction = other.direction;
        n_profiles = other.n_profiles;
        proplist = other.proplist;
        latency_offset = other.latency_offset;
        enabled = enable;
        type = getDevicePortType(cardName, other.name);

        profiles.clear();
        for (int i = 0; i < n_profiles; ++i) {
            profiles.push_back(std::make_shared<PaCardProfileInfo>(*other.profiles[i]));
        }

        profiles2.clear();
        for (int i = 0; i < n_profiles; ++i) {
            profiles2.push_back(std::make_shared<PaCardProfileInfo2>(*other.profiles2[i]));
        }

    }

} PaCardPortInfo;

typedef struct PaCardInfo {
    uint32_t index;                      /**< Index of this card */
    std::string name;                    /**< Name of this card */
    std::string deviceDesc;              /**< Decription of this card */
    std::string deviceFactor;            /** For devices: form factor if applicable.*/
    uint32_t owner_module;               /**< Index of the owning module, or PA_INVALID_INDEX. */
    std::string driver;                  /**< Driver name */
    uint32_t n_profiles;                 /**< Number of entries in profile array */
    std::shared_ptr<PaCardProfileInfo> profiles;      /**< \deprecated Superseded by profiles2 */
    std::shared_ptr<PaCardProfileInfo> active_profile; /**< \deprecated Superseded by active_profile2 */
    pa_proplist *proplist;               /**< Property list */
    uint32_t n_ports;                    /**< Number of entries in port array */
    std::vector<std::shared_ptr<PaCardPortInfo>> ports;           /**< Array of pointers to ports, or NULL. Array is terminated by an entry set to NULL. */
    std::vector<std::shared_ptr<PaCardProfileInfo2>> profiles2;    /**< Array of pointers to available profiles, or NULL. Array is terminated by an entry set to NULL. \since 5.0 */
    std::shared_ptr<PaCardProfileInfo2> active_profile2; /**< Pointer to active profile in the array, or NULL. \since 5.0 */

    std::list<std::shared_ptr<DeviceInfo>> deviceList {};

    void operator =(const pa_card_info& other);
} PaCardInfo;

typedef struct PaServerInfo {
    std::string user_name;              /**< User name of the daemon process */
    std::string host_name;              /**< Host name the daemon is running on */
    std::string server_version;         /**< Version string of the daemon */
    std::string server_name;            /**< Server package name (usually "pulseaudio") */
    pa_sample_spec sample_spec;         /**< Default sample specification */
    std::string default_sink_name;      /**< Name of default sink. */
    std::string default_source_name;    /**< Name of default source. */
    uint32_t cookie;                    /**< A random cookie for identifying this instance of PulseAudio. */
    pa_channel_map channel_map;         /**< Default channel map. \since 0.9.15 */

    void operator =(const pa_server_info& other)
    {
        user_name = other.user_name ? other.user_name : "";
        host_name = other.host_name ? other.host_name : "";
        server_version = other.server_version ? other.server_version : "";
        server_name = other.server_name ? other.server_name : "";
        sample_spec = other.sample_spec;
        channel_map = other.channel_map;
        default_sink_name = other.default_sink_name ? other.default_sink_name : "";
        default_source_name = other.default_source_name ? other.default_source_name : "";
        cookie = other.cookie;
        channel_map = other.channel_map;
    }
} PaServerInfo;


typedef struct PaSinkInputInfo {
    uint32_t index;                      /**< Index of the sink input */
    std::string name;                    /**< Name of the sink input */
    uint32_t owner_module;               /**< Index of the module this sink input belongs to, or PA_INVALID_INDEX when it does not belong to any module. */
    uint32_t client;                     /**< Index of the client this sink input belongs to, or PA_INVALID_INDEX when it does not belong to any client. */
    uint32_t sink;                       /**< Index of the connected sink */
    pa_sample_spec sample_spec;          /**< The sample specification of the sink input. */
    pa_channel_map channel_map;          /**< Channel map */
    pa_cvolume volume;                   /**< The volume of this sink input. */
    pa_usec_t buffer_usec;               /**< Latency due to buffering in sink input, see pa_timing_info for details. */
    pa_usec_t sink_usec;                 /**< Latency of the sink device, see pa_timing_info for details. */
    std::string resample_method;         /**< The resampling method used by this sink input. */
    std::string driver;                  /**< Driver name */
    int mute;                            /**< Stream muted \since 0.9.7 */
    pa_proplist *proplist;               /**< Property list \since 0.9.11 */
    int corked;                          /**< Stream corked \since 1.0 */
    int has_volume;                      /**< Stream has volume. If not set, then the meaning of this struct's volume member is unspecified. \since 1.0 */
    int volume_writable;                 /**< The volume can be set. If not set, the volume can still change even though clients can't control the volume. \since 1.0 */
    pa_format_info *format;              /**< Stream format information. \since 1.0 */

    void operator =(const pa_sink_input_info& other)
    {
        index = other.index;
        name = other.name ? other.name : "";
        owner_module = other.owner_module;
        client = other.client;
        sink = other.sink;
        sample_spec = other.sample_spec;
        channel_map = other.channel_map;
        volume = other.volume;
        buffer_usec = other.buffer_usec;
        sink_usec = other.sink_usec;
        resample_method = other.resample_method ? other.resample_method : "";
        driver = other.driver ? other.driver : "";
        mute = other.mute;
        proplist = other.proplist;
        corked = other.corked;
        has_volume = other.has_volume;
        volume_writable = other.volume_writable;
        format = other.format;
    }

} PaSinkInputInfo;

typedef struct PaSourceOutputInfo {
    uint32_t index;                      /**< Index of the source output */
    std::string name;                    /**< Name of the source output */
    uint32_t owner_module;               /**< Index of the module this source output belongs to, or PA_INVALID_INDEX when it does not belong to any module. */
    uint32_t client;                     /**< Index of the client this source output belongs to, or PA_INVALID_INDEX when it does not belong to any client. */
    uint32_t source;                     /**< Index of the connected source */
    pa_sample_spec sample_spec;          /**< The sample specification of the source output */
    pa_channel_map channel_map;          /**< Channel map */
    pa_usec_t buffer_usec;               /**< Latency due to buffering in the source output, see pa_timing_info for details. */
    pa_usec_t source_usec;               /**< Latency of the source device, see pa_timing_info for details. */
    std::string resample_method;         /**< The resampling method used by this source output. */
    std::string driver;                  /**< Driver name */
    pa_proplist *proplist;               /**< Property list \since 0.9.11 */
    int corked;                          /**< Stream corked \since 1.0 */
    pa_cvolume volume;                   /**< The volume of this source output \since 1.0 */
    int mute;                            /**< Stream muted \since 1.0 */
    int has_volume;                      /**< Stream has volume. If not set, then the meaning of this struct's volume member is unspecified. \since 1.0 */
    int volume_writable;                 /**< The volume can be set. If not set, the volume can still change even though clients can't control the volume. \since 1.0 */
    pa_format_info *format;              /**< Stream format information. \since 1.0 */

    void operator =(const pa_source_output_info& other)
    {
        index = other.index;
        name = other.name ? other.name : "";
        owner_module = other.owner_module;
        client = other.client;
        source = other.source;
        sample_spec = other.sample_spec;
        channel_map = other.channel_map;
        volume = other.volume;
        buffer_usec = other.buffer_usec;
        source_usec = other.source_usec;
        resample_method = other.resample_method ? other.resample_method : "";
        driver = other.driver ? other.driver : "";
        proplist = other.proplist;
        corked = other.corked;
        mute = other.mute;
        has_volume = other.has_volume;
        volume_writable = other.volume_writable;
        format = other.format;
    }
} PaSourceOutputInfo;


typedef struct CardInfo {
    PaCardInfo card;
    std::list<std::shared_ptr<IStream>> sinks;
    std::list<std::shared_ptr<IStream>> sources;

    std::string toString() const;
} CardInfo;

typedef struct PaDataInfo
{
    std::list<std::shared_ptr<CardInfo>> cardInfoList;
    std::list<std::shared_ptr<IStream>> sinkInputList;
    std::list<std::shared_ptr<IStream>> sourceOutputList;
    std::list<PaModuleInfo> moduleInfoList;
    PaServerInfo serverInfo;

    std::shared_ptr<PaSinkStream> defaultSink;
    std::shared_ptr<PaSourceStream> defaultSource;

    mutable std::mutex m_mtx;
    std::condition_variable m_cond;
    bool m_operation = false;
    std::string toString() const;
} PaDataInfo;


uint32_t valueToPaVolume(int);  //滑动条值转换成音量
uint32_t paVolumeToValue(int);  //音量值转换成滑动条值
uint32_t paCvolumeToValue(const pa_cvolume&);
uint32_t paCvolumeToPaValue(const pa_cvolume&);

}
#endif // PADATATYPE_H
