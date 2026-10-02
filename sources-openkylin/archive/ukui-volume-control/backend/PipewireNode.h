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

//#include <QObject>
//#include <QIcon>
//#include <QtQml>

#include <string>
#include <spa/utils/defs.h>
#include <spa/debug/pod.h>
#include <spa/node/node.h>
#include <pipewire/node.h>
#include <variant>
#include <gio/gio.h>
#include <unordered_map>

namespace UkuiAudioFramwork
{
class Pipewire;
class PipewireProfiler;
class driver;

class PipewireNode
{
public:
    enum NodeType {
        NodeTypeNone = 0,
        NodeTypeInput,
        NodeTypeOutput,
        NodeTypeSink,
        NodeTypeSource,
    };

    enum MediaType {
        MediaTypeNone = 0,
        MediaTypeAudio,
        MediaTypeVideo,
        MediaTypeMidi,
    };

    struct driver {
            int64_t count = 0;
            float cpu_load[3] = {0,0,0};
            struct spa_io_clock clock = {};
            uint32_t xrun_count = 0;
    } info;

    struct measurement {
            int32_t index = 0;
            int32_t status = 0;
            int64_t quantum = 0;
            int64_t prev_signal = 0;
            int64_t signal = 0;
            int64_t awake = 0;
            int64_t finish = 0;
            struct spa_fraction latency = {0,0};
    } measurement;

    uint32_t errors = 0;
    int32_t last_error_status = 0;
private:
    void idChanged();
    void nameChanged();
    void nodeNameChanged();
    void nodeDescriptionChanged();
    void categoryChanged();
    void mediaClassChanged();
    void nodeTypeChanged();
    void mediaTypeChanged();
    void driverChanged();
    void activeChanged();
    void waitingChanged();
    void busyChanged();
    void quantumChanged();
    void rateChanged();
    void errorChanged();
    void xrunChanged();
    void volumeChanged(float);
    void stateChanged();

protected:
    Pipewire *pipewire = nullptr;

    uint32_t m_id = 0;
    pw_node_state m_state = pw_node_state::PW_NODE_STATE_CREATING;
    std::string m_name;
    std::string m_node_name;
    std::string m_node_description;
    std::string m_category;
    std::string m_media_class;
    NodeType m_node_type = NodeTypeNone;
    MediaType m_media_type = MediaTypeNone;
    struct ::pw_node *m_spa_node = nullptr;
    struct spa_node_info m_spa_node_info {};
//    QHash<std::string, std::string> m_properties;
    std::unordered_map<std::string, std::string> m_properties;

    spa_hook object_listener;
    uint32_t _props_seq = 0;


//    struct measurement {
//            int32_t index = 0;
//            int32_t status = 0;
//            int64_t quantum = 0;
//            int64_t prev_signal = 0;
//            int64_t signal = 0;
//            int64_t awake = 0;
//            int64_t finish = 0;
//            struct spa_fraction latency = {0,0};
//    } measurement;

//    struct driver {
//            int64_t count = 0;
//            float cpu_load[3] = {0,0,0};
//            struct spa_io_clock clock = {};
//            uint32_t xrun_count = 0;
//    } info;

    PipewireNode *m_driver = nullptr;
//    uint32_t errors = 0;
//    int32_t last_error_status = 0;

    float m_volume = 0.0;

public:
    PipewireNode() = default;
    explicit PipewireNode(Pipewire *parent, uint32_t id, const struct spa_dict *props);
    virtual ~PipewireNode();

    void _node_event_info(const pw_node_info *info);
    void _event_param(int seq, uint32_t id, uint32_t index, uint32_t next, const spa_pod *param);

    int id() const { return m_id; }
    uint32_t id_u32() const { return m_id; }
    std::string name() const { return m_name; }
    std::string nodeName() const { return m_node_name; }
    std::string nodeDescription() const { return m_node_description; }
    std::string category() const { return m_category; }
    std::string mediaClass() const { return m_media_class; }
    NodeType nodeType() const { return m_node_type; }
    MediaType mediaType() const { return m_media_type; }
    PipewireNode *driver() { return m_driver != this ? m_driver : nullptr; }
    bool active() const { return measurement.status == 3; }
    double waiting() const { return (measurement.awake - measurement.signal) / 1000000000.f; }
    double busy() const { return (measurement.finish - measurement.awake) / 1000000000.f; }
    int quantum() const {
        if (m_driver == this) {
            return info.clock.duration * info.clock.rate.num;
        } else {
            return measurement.latency.num;
        }
    }
    int rate() const {
        if (m_driver == this) {
            return info.clock.rate.denom;
        } else {
            return measurement.latency.denom;
        }
    }
    int error() const { return errors; }
    int xrun() const { return info.xrun_count; }
    float volume() const { return m_volume; }
    std::string state() const {
        switch (m_state) {
            case PW_NODE_STATE_ERROR: return "ERROR";
            case PW_NODE_STATE_CREATING: return "CREATING";
            case PW_NODE_STATE_SUSPENDED: return "SUSPENDED";
            case PW_NODE_STATE_IDLE: return "IDLE";
            case PW_NODE_STATE_RUNNING: return "RUNNING";
            default: return "UNKNOWN";
        }
    }

//    Q_INVOKABLE QString formatPercentage(float val, float quantum) const;
//    Q_INVOKABLE QIcon activeIcon(bool active) const;

//    Q_INVOKABLE virtual bool isAlsa() const { return false; }

//    /// Volume has to be between 0.0 and 1.0 (included)
//    Q_INVOKABLE void setVolume(float volume);

    std::string formatPercentage(float val, float quantum) const;
    //    Q_INVOKABLE QIcon activeIcon(bool active) const;

    virtual bool isAlsa() const { return false; }

    // Volume has to be between 0.0 and 1.0 (included)
    void setVolume(float volume);


    // add
    void setDriver(PipewireNode *newDriver);
    void setMeasurement(const struct measurement &measure);
    void setInfo(const struct driver &info);

protected:

    float _quantum()
    {
        if (info.clock.rate.denom)
            return info.clock.duration * info.clock.rate.num / info.clock.rate.denom;
        else
            return 0;
    }

//    void setDriver(PipewireNode *newDriver);
//    void setMeasurement(const struct measurement &measure);
//    void setInfo(const struct driver &info);

//    Q_INVOKABLE QVariant property(const char* key);
//    Q_INVOKABLE void setProperty(const char* key, QVariant value);

    GVariant* property(const char* key);
    void setProperty(const char* key, GVariant* value);
    void setProperties(struct spa_pod *properties);
    void enumParams();

    friend class QPipewireProfiler;
};
}
//Q_DECLARE_METATYPE(QPipewireNode*);
