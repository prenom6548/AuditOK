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

#include "PipewireNode.h"

#include <spa/param/props.h>
#include <spa/param/param.h>
#include <spa/pod/vararg.h>
#include <spa/pod/builder.h>
#include <spa/debug/pod.h>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

#include "PipewireManager1.h"
#include "utils.h"


namespace UkuiAudioFramwork
{
template <typename T>
T clamp(const T v, const T low, const T high)
{
    if (v < low)
        return low;
    if (v > high)
        return high;
    return v;
}

#define PIPEWIRE_CAST(x) PipewireNode* _this = static_cast<PipewireNode*>(x);

void node_event_info(void *data, const pw_node_info *info)
{
    PIPEWIRE_CAST(data);
    _this->_node_event_info(info);
}

void PipewireNode::_node_event_info(const pw_node_info* info)
{
    // message when it's updated
    // qDebug() << "node_event_info: ";
    const struct spa_dict_item *item;

    if (info->change_mask == PW_NODE_CHANGE_MASK_ALL) {
        //TODO
        // qDebug() << "CHANGE ALL";
    }
    if (info->change_mask & PW_NODE_CHANGE_MASK_INPUT_PORTS ) {
        //TODO
        // qDebug() << "CHANGE INPUT PORTS";
    }
    if (info->change_mask & PW_NODE_CHANGE_MASK_OUTPUT_PORTS) {
        //TODO
        // qDebug() << "CHANGE OUTPUT PORTS";
    }
    if (info->change_mask & PW_NODE_CHANGE_MASK_STATE) {
        m_state = info->state;
//        emit stateChanged();
        // qDebug() << "CHANGE STATE";
    }
    if (info->change_mask & PW_NODE_CHANGE_MASK_PROPS) {
        spa_dict_for_each(item, info->props) {
            //volume is not here....
            const std::string key = item->key;
            const std::string value = item->value;
            m_properties[key] = value;
            std::cout << "node_" << this->m_name << ":[" << key << "]=" << value << std::endl;
            //emit propertyChanged(key, value);
        }
        // qDebug() << "CHANGE PROPS";
    }
    if (info->change_mask & PW_NODE_CHANGE_MASK_PARAMS) {
        enumParams();
        // qDebug() << "CHANGE PARAMS";
    }
}


void event_param(void *data,
                 int seq,
                 uint32_t id,
                 uint32_t index,
                 uint32_t next,
                 const spa_pod *param)
{
    PIPEWIRE_CAST(data);
    _this->_event_param(seq, id, index, next, param);

    //volume is here!
}

void PipewireNode::_event_param(int seq, uint32_t id, uint32_t index, uint32_t next, const spa_pod* param)
{
    const spa_pod_prop *prop;
    const spa_pod_object *obj = (const struct spa_pod_object*)param;

    prop = spa_pod_find_prop(param, nullptr, SPA_PROP_channelVolumes);
    if (prop != nullptr) {
        assert(spa_pod_is_array(&prop->value));
        const spa_pod_array *volumes = (const spa_pod_array*) &prop->value;

        uint32_t n_channels = 0;
        uint32_t type = SPA_POD_ARRAY_VALUE_TYPE(volumes);
        assert(SPA_TYPE_Float == type);
        float *data = (float*) spa_pod_get_array((const spa_pod*) volumes, &n_channels);
        m_volume = data[0];
        std::cout << "m_node_name:" << m_node_name << " volume changed, volume:" << m_volume
                  << " m_node_description:" << m_node_description << std::endl;
//        emit volumeChanged(m_volume);
    }
}


static const struct pw_node_events node_events = {
    .version = PW_VERSION_NODE_EVENTS,
    .info = &node_event_info,
    .param = &event_param
};


PipewireNode::PipewireNode(Pipewire *parent, uint32_t id, const struct spa_dict *props)
    : /*QObject(parent)
    ,*/ pipewire(parent)
    , m_id(id)
    , m_node_type(NodeTypeNone)
    , m_media_type(MediaTypeNone)
    , m_driver(this)

{
    const char* nodeName = spa_dict_lookup(props, PW_KEY_NODE_NAME);
    m_node_name = nodeName != nullptr ? nodeName : "";
    const char* nodeDescription = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
    m_node_description = nodeDescription != nullptr ? nodeDescription : "";
    std::cout << "Pipewire Node.name: " << m_node_name << " node desc: " << m_node_description << std::endl;
    // Find name
    const char* str;
    if ((str = spa_dict_lookup(props, PW_KEY_NODE_NICK)) == nullptr &&
        (str = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION)) == nullptr &&
        (str = spa_dict_lookup(props, PW_KEY_NODE_NAME)) == nullptr)
    {
        str = spa_dict_lookup(props, PW_KEY_APP_NAME);
    }

    if (str == nullptr) {
        m_name = std::to_string(id);
    } else {
        m_name = str;
    }

    // Find node type
    if ((str = spa_dict_lookup(props, PW_KEY_MEDIA_CATEGORY)) != nullptr) {
        m_category = str;
    } else {
        m_category = "";
    }

    // Find node type
    if ((str = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS)) != nullptr) {
        m_media_class = str;


        if (std::string::npos != m_media_class.find("Audio"))
            m_media_type = MediaTypeAudio;
        else if (std::string::npos != m_media_class.find("Video"))
            m_media_type = MediaTypeVideo;
        else if (std::string::npos != m_media_class.find("Midi"))
            m_media_type = MediaTypeMidi;
        else
            m_media_type = MediaTypeNone;
    } else {
        m_media_class = "";
    }

    if ((!m_category.empty()) && std::string::npos != m_category.find("Duplex")) {
        m_node_type = NodeTypeNone;
    } else if (!m_media_class.empty()) {
        if (std::string::npos != m_media_class.find("Sink")) {
            m_node_type = NodeTypeSink;
        } else if (std::string::npos != m_media_class.find("Input")) {
            m_node_type = NodeTypeInput;
        } else if (std::string::npos != m_media_class.find("Source")) {
            m_node_type = NodeTypeSource;
        } else if (std::string::npos != m_media_class.find("Output")) {
            m_node_type = NodeTypeOutput;
        }
    } else {
        m_node_type = NodeTypeNone;
    }

    m_spa_node = static_cast<pw_node*>(
        pw_registry_bind(pipewire->registry, m_id, PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, 0));

    const struct spa_dict_item *item;
    spa_dict_for_each(item, props) {
         std::cout << "Property[" << item->key << "]=" << item->value << std::endl;
        m_properties[item->key] = item->value;
    }

//    struct spa_pod_object *obj = (struct spa_pod_object *) m_spa_node;
//    struct spa_pod_prop *prop;
//    SPA_POD_OBJECT_FOREACH(obj, prop) {
//        switch (prop->key) {
//        case SPA_PROP_volume:
//            float volume;
//            if (spa_pod_get_float(&prop->value, &volume) < 0)
//                continue;
//            if (volume == m_volume)
//                continue;
//            m_volume = volume;
////            emit volumeChanged(m_volume);
//            break;
//        }
//        const spa_pod *pod = &prop->value;
//        std::cout << "KEY: " << prop->key << "=" << prop->value << std::endl;
//    }
    pw_proxy_add_object_listener((pw_proxy*) this->m_spa_node, &object_listener, &node_events, this);
    // enumParams();
}

PipewireNode::~PipewireNode()
{
    if (m_spa_node != nullptr) {
        pw_proxy_destroy((pw_proxy *) m_spa_node);
    }
}

std::string PipewireNode::formatPercentage(float val, float quantum) const
{
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%5.2f", quantum == 0.0f ? 0.0f : val/quantum);
    return std::string(buffer);
}

//QIcon PipewireNode::activeIcon(bool active) const
//{
//    if(active) {
//        return QIcon::fromTheme("media-playback-start");
//    } else {
//        return QIcon::fromTheme("media-playback-pause");
//    }
//}

void PipewireNode::setDriver(PipewireNode* newDriver)
{
    if (newDriver == m_driver) return;
    m_driver = newDriver;
//    emit driverChanged();
}

void PipewireNode::setMeasurement(const struct PipewireNode::measurement &measure)
{
    const struct measurement old = this->measurement;
    this->measurement = measure;
//    if (old.status != measure.status) emit activeChanged();
//    if (old.signal != measure.signal || old.awake != measure.awake)
//        emit waitingChanged();
//    if (old.finish != measure.finish || old.awake != measure.awake)
//        emit busyChanged();
//    if (m_driver != this && old.latency.num != measure.latency.num)
//        emit quantumChanged();
//    if (m_driver != this && old.latency.denom != measure.latency.denom)
//        emit rateChanged();
}

void PipewireNode::setInfo(const struct PipewireNode::driver &info)
{
    const struct driver old = this->info;
    this->info = info;
//    if (m_driver == this &&
//        (old.clock.duration != info.clock.duration || old.clock.rate.num != info.clock.rate.num))
//        emit quantumChanged();
//    if (m_driver == this && old.clock.rate.denom != info.clock.rate.denom)
//        emit rateChanged();
//    if (old.xrun_count != info.xrun_count)
//        emit xrunChanged();
}

GVariant* PipewireNode::property(const char *key)
{
    return g_variant_new("s", m_properties[key].c_str());
}

void PipewireNode::setProperty(const char *key, GVariant* value)
{
    throw std::runtime_error("not implemented");
}

void PipewireNode::setProperties(struct spa_pod *properties)
{
    spa_debug_pod(0, nullptr, properties);
    int res = pw_node_set_param(m_spa_node, SPA_PARAM_Props, 0, properties);
    if (res < 0) {
        std::ostringstream err_msg;
        err_msg << "Got set_property error \"" << res << '"';
        throw std::runtime_error(err_msg.str());
    }
}

void PipewireNode::setVolume(float volume)
{
    volume = clamp(volume, 0.0f, 1.0f);
    struct spa_pod_builder builder = {};
    unsigned char buffer[512];
    spa_pod_builder_init(&builder, buffer, sizeof(buffer));

    struct spa_pod *props = static_cast<spa_pod*>(
        spa_pod_builder_add_object(&builder,
                                   SPA_TYPE_OBJECT_Props, 0,
                                   SPA_PROP_volume, SPA_POD_Float(volume)));


    this->setProperties(props);

    m_volume = volume;
//    emit volumeChanged(m_volume);
}


void PipewireNode::enumParams()
// static bool do_enum_params(struct data *data, const char *cmd, char *args, char **error)
{
    spa_pod *filter = nullptr;
    pw_node_enum_params(m_spa_node, _props_seq++, SPA_PARAM_Props, 0, 0, filter);
    // pw_node_enum_params(m_spa_node, _props_seq++, SPA_PARAM_Route, 0, 0, filter);
    // pw_node_set_param();
}

}
