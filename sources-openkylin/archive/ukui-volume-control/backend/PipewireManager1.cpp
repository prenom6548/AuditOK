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

#include "PipewireManager1.h"


#include <spa/utils/result.h>
#include <spa/utils/defs.h>
#include <spa/pod/parser.h>
#include <spa/debug/pod.h>

#include <pipewire/pipewire.h>
#include <pipewire/impl.h>
#include <pipewire/extensions/profiler.h>

#include <csignal>
#include <stdexcept>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstdlib>

#include "AudioContext.h"
#include "PipewireSettings.h"
#include "PipewireClient.h"
#include "PipewireNode.h"
#include "PipewireLink.h"
#include "PipewireAlsaNode.h"
#include "PipewirePort.h"
#include "PipewireDevice.h"
//#include <QDebug>

namespace UkuiAudioFramwork
{
#define PIPEWIRE_CAST(x) Pipewire* _this = static_cast<Pipewire*>(x);

static inline bool streq(const char *s1, const char *s2)
{
    return SPA_LIKELY(s1 && s2) ? strcmp(s1, s2) == 0 : s1 == s2;
}


void Pipewire::round_trip()
{
    resync();

    // The event is collected by callback _on_core_done and
    // when proper, it sets round_trip_done to true
    while(!round_trip_done) {
        pw_main_loop_run(loop);
    }
}

std::string Pipewire::formatTime(double val)
{
    char buffer[64];
    val *= 1000000000.0f;

    if (val < 1000000llu) {
        val /= 1000.0f;
        std::snprintf(buffer, sizeof(buffer), "%5.1fµs", val);
    } else if (val < 1000000000llu) {
        val /= 1000000.0f;
        std::snprintf(buffer, sizeof(buffer), "%5.1fms", val);
    } else {
        val /= 1000000000.0f;
        std::snprintf(buffer, sizeof(buffer), "%5.1fs", val);
    }
    return std::string(buffer);
}

void Pipewire::resync()
{
    round_trip_done = false;

    // This is async, will trigger a DONE event
    // with `seq` number the number just returned by this function
    sync = pw_core_sync(core, PW_ID_CORE, sync);
}

//-----------------------------------------------------------------------------
static void do_quit(void *userdata, int /*signal_number*/)
{
    PIPEWIRE_CAST(userdata);
    _this->_quit();
}

void Pipewire::_quit()
{
    pw_main_loop_quit(loop);
//    emit quit();
}

//-----------------------------------------------------------------------------
static void on_core_done(void *data, uint32_t id, int seq)
{
    PIPEWIRE_CAST(data);
    _this->_on_core_done(id, seq);
}

void Pipewire::_on_core_done(uint32_t id, int seq)
{
//    std::cout << "_on_core_done(" << id << ',' << seq << ')' << std::endl;
    if (id == PW_ID_CORE && sync == seq) {
        round_trip_done = true;
        pw_main_loop_quit(loop);
    }
}

static void on_core_error(void *data, uint32_t id, int seq, int res, const char *message)
{
    PIPEWIRE_CAST(data);
    _this->_on_core_error(id, seq, res, message);
}

void Pipewire::_on_core_error(u_int32_t id, int seq, int res, const char *message)
{
    pw_log_error("error id:%u seq:%d res:%d (%s): %s",
                    id, seq, res, spa_strerror(res), message);

    if (id == PW_ID_CORE && res == -EPIPE) {
         pw_main_loop_quit(loop);
    }
}

static const pw_core_events core_events = {
    .version = PW_VERSION_CORE_EVENTS,
    .info = nullptr,
    .done = on_core_done,
    .ping = nullptr,
    .error = on_core_error,
    .remove_id = nullptr,
    .bound_id = nullptr,
    .add_mem = nullptr,
    .remove_mem = nullptr
};

//-----------------------------------------------------------------------------
static void registry_event(void *data,
                           uint32_t id,
                           uint32_t permissions,
                           const char *type,
                           uint32_t version,
                           const struct spa_dict *props)
{
    PIPEWIRE_CAST(data);
    _this->_registry_event(id, permissions, type, version, props);
}

static void registry_event_remove(void *data, uint32_t id)
{
    PIPEWIRE_CAST(data);
    _this->_registry_event_remove(id);
}

void Pipewire::_registry_event(uint32_t id,
                                uint32_t permissions,
                                const char *type,
                                uint32_t version,
                                const struct spa_dict *props)
{
     std::cout << "object: id(" << id << ") type(" << type << '/' << version << ')' << std::endl;

//    emit registryObject(id, permissions, type, version, props);

    if (strcmp(type, PW_TYPE_INTERFACE_Client) == 0) {
        PipewireClient *pw_client = new PipewireClient(this, id, props);
//        emit clientChanged();

        // Shot only once when data has arrived
//        QMetaObject::Connection *const connection = new QMetaObject::Connection;
//        *connection = connect(pw_client, &QPipewireClient::propertiesChanged, [this, pw_client, connection]() {
//            if (this->isPipewireMediaSession()) {
//                // creating alsa properties when media-session is not installed could crash the application
//                alsa_properties = new AlsaProperties(pw_client, this);
//                emit alsaPropertiesChanged();
//            }
//            // delete so the connection is not shot twice
//            delete connection;
//        });
		pw_clients.push_back(pw_client);
    }
    else if (strcmp(type, PW_TYPE_INTERFACE_Metadata) == 0)
    {
        const char *metadata_name;
        metadata_name = spa_dict_lookup(props, PW_KEY_METADATA_NAME);
        if (metadata_name != nullptr)
        {
            if (this->pw_settings == nullptr &&
                    streq(metadata_name, "settings"))
            {
                pw_settings = new PipewireSettings(this, id, props);
//                emit settingsChanged();
            } else {
                // qWarning() << "Ignoring metadata \"" << metadata_name << '"';
            }
        }
    }
    else if (streq(type, PW_TYPE_INTERFACE_Profiler))
    {
        if (pw_profiler != nullptr) {
            // qWarning() << "Ignoring profiler " << id << ": already attached";
            return;
        }
        pw_profiler = new PipewireProfiler(this, id, props);
//        emit profilerChanged();
    }
    else if (streq(type, PW_TYPE_INTERFACE_Node))
    {
        PipewireNode *node;
        const std::string str = spa_dict_lookup(props, PW_KEY_NODE_NAME) ? spa_dict_lookup(props, PW_KEY_NODE_NAME) : "";

        if (str.starts_with("alsa_input.") || str.starts_with("alsa_output.")) {
            node = new PipewireAlsaNode(this, id, props);
        } else {
            node = new PipewireNode(this, id, props);
        }
        m_nodes.push_back(node);
        std::cout << "Adding node id(" << id << "): " << node->name()
                  << " nodetype (" << node->nodeType() << ")" << std::endl;
//        emit nodesChanged();
    }
    else if (streq(type, PW_TYPE_INTERFACE_Link))
    {
        PipewireLink* link = new PipewireLink(this, id, props);
        m_links.push_back(link);
        std::cout << "Adding link id(" << id << "): " << std::endl;
//	    emit linksChanged();
    }
    else if (streq(type, PW_TYPE_INTERFACE_Port))
    {
        PipewirePort *port = new PipewirePort(this, id, props);
        m_ports.push_back(port);
        std::cout << "Adding port id(" << id << "): " << std::endl;
//	    emit portsChanged();
    }
    else if (streq(type, PW_TYPE_INTERFACE_Device)) {
        PipewireDevice *device = new PipewireDevice(this, id, props);
        m_devices.push_back(device);
        std::cout << "Adding device id(" << id << "): " << std::endl;
    }
}

void Pipewire::_registry_event_remove(uint32_t id)
{
    std::cout << "Attempting to remove id(" << id << ")" << std::endl;
    for(int i=0; i<m_nodes.size(); i++) {
        PipewireNode *candidate = (m_nodes)[i];
        if (candidate && candidate->id_u32() == id) {
            PipewireNode *node = candidate;
            // ALL interfaces that are not NODES are ignored here.
            std::cout << "Removing id(" << id << ":" << node->id() << "): " << node->name() << std::endl;
            m_nodes.erase(m_nodes.begin()+ i);
//            emit nodesChanged();
//            node->deleteLater();
            return;
        }
    }

    for(int i=0; i<m_links.size(); i++) {
        PipewireLink *candidate = m_links[i];
        if (candidate && candidate->id_u32() == id) {
            PipewireLink* link = candidate;
            m_links.erase(m_links.begin() + i);
//            emit linksChanged();
//            link->deleteLater();
            return;
        }
    }

}

static const pw_registry_events registry_events = {
    .version = PW_VERSION_REGISTRY_EVENTS,
    .global = registry_event,
    .global_remove = registry_event_remove,
};

//-----------------------------------------------------------------------------
Pipewire::Pipewire(int *argc, char **argv[])
{
    spa_zero(core_listener);
    spa_zero(registry_listener);

    pw_init(argc, argv);

    std::cout << "Compiled with libpipewire " << pw_get_headers_version() << std::endl;
    std::cout << "Linked   with libpipewire " << pw_get_library_version() << std::endl;

    loop = pw_main_loop_new(nullptr);
    if (loop == nullptr) {
        throw std::runtime_error("Error initializing Pipewire mainloop");
    }

    pw_loop_add_signal(pw_main_loop_get_loop(loop), SIGINT, do_quit, this);
    pw_loop_add_signal(pw_main_loop_get_loop(loop), SIGTERM, do_quit, this);

    context = pw_context_new(pw_main_loop_get_loop(loop), nullptr, 0);
    if (context == nullptr) {
        throw std::runtime_error("Error creating Pipewire context");
    }

    // Need this line or the profiler will not load.
    pw_context_load_module(context, PW_EXTENSION_MODULE_PROFILER, nullptr, nullptr);

    //TODO remote should be? (empty string seems to work correctly)
    const char *remote = "";
    core = pw_context_connect(context,
                              nullptr,//pw_properties_new(PW_KEY_REMOTE_NAME, remote, nullptr),
                              0);
    if (core == nullptr) {
        throw std::runtime_error("Can't connect to pipewire");
    }

    pw_core_add_listener(core,
                         &core_listener,
                         &core_events,
                         this);

    registry = pw_core_get_registry(core,
                                    PW_VERSION_REGISTRY,
                                    0);
    pw_registry_add_listener(registry,
                             &registry_listener,
                             &registry_events,
                             this);

    pipewire_media_session = new SystemdService("pipewire-media-session", true);
    wireplumber_service = new SystemdService("wireplumber", true);

    // QT stuff
//    connect(m_nodes, &QPipewireNodeListModel::layoutChanged, this, [this]() {
//        emit nodesChanged();
//    });
}

Pipewire::~Pipewire()
{
    {
        std::lock_guard<std::mutex> lock(m_moduleMtx);
        for (auto& [name, module] : m_loadedModules) {
            if (module)
                pw_impl_module_destroy(module);
        }
        m_loadedModules.clear();
    }

    delete pipewire_media_session;

    if (pw_settings != nullptr) {
        delete pw_settings;
    }
    for (PipewireClient* pw_client: pw_clients) {
        delete pw_client;
    }
    if (alsa_properties != nullptr) {
        delete alsa_properties;
    }
    if (pw_profiler != nullptr) {
        delete pw_profiler;
    }
//    PipewireNode *node;
    for(const auto& node : m_nodes) {
        delete node;
    }
//    delete m_nodes;
    pw_proxy_destroy((struct pw_proxy*) registry);
    pw_core_disconnect(core);
    pw_context_destroy(context);
    pw_main_loop_destroy(loop);
    pw_deinit();
}

void Pipewire::init()
{
    std::atomic<bool> stop(false);
    std::thread worker([this, &stop]() {
        while (!stop) {
            round_trip();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    worker.join();
}

bool Pipewire::connect()
{
    // TODO
    return true;
}

// Interface
void Pipewire::loadModule(const std::string& name, const std::string& param)
{
    const char* args = param.empty() ? nullptr : param.c_str();
    ::pw_impl_module* module = pw_context_load_module(context, name.c_str(), args, nullptr);
    if (!module) {
        AUDIO_ERROR("Failed to load module {}.", name);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(m_moduleMtx);
        m_loadedModules[name] = module;
    }
    AUDIO_DEBUG("Module {} loaded successfully.", name);

    if (name == "libpipewire-module-combine-stream") {
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            resync();
            std::system("pactl set-default-sink combined 2>/dev/null");
            deviceAdjust(NodeType::OUTPUT);
        }).detach();
    }
}

void Pipewire::unloadModule(const std::string& name, bool restoreMasterDevice)
{
    ::pw_impl_module* module = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_moduleMtx);
        auto it = m_loadedModules.find(name);
        if (it == m_loadedModules.end() && name == "module-combine-sink") {
            it = m_loadedModules.find("libpipewire-module-combine-stream");
        }
        if (it != m_loadedModules.end()) {
            module = it->second;
            m_loadedModules.erase(it);
        }
    }

    if (!module) {
        AUDIO_ERROR("Unload {} module failed, not found such module.", name);
        return;
    }

    pw_impl_module_destroy(module);
    AUDIO_DEBUG("Unload module {}.", name);

    if (restoreMasterDevice &&
        (name == "module-combine-sink" || name == "libpipewire-module-combine-stream")) {
        deviceAdjust(NodeType::OUTPUT);
    }
}

// Method
void Pipewire::setDetailVolume(const NodeType& type, const std::string& name, const std::string& dev, int volume)
{
    // TODO
}

int Pipewire::getDetailVolume(const NodeType& type, const std::string& name, const std::string& dev) const
{
    // TODO
    return 100;
}

void Pipewire::setVolume(const NodeType& type, int idx, int volume)
{
    // TODO
}

int Pipewire::getVolume(const NodeType& type, int idx) const
{
    // TODO
    return 100;
}

void Pipewire::setDetailMute(const NodeType& type, const std::string& name, const std::string& dev, bool mute)
{
    // TODO
}

bool Pipewire::getDetailMute(const NodeType& type, const std::string& name, const std::string& dev) const
{
    // TODO
    return false;
}

void Pipewire::setBalance(const NodeType& type, double v)
{
    // TODO
}

double Pipewire::getBalance(const NodeType& type) const
{
    // TODO
    return 0.0;
}

void Pipewire::setMute(const NodeType& type, int idx, bool mute)
{
    // TODO
}

bool Pipewire::getMute(const NodeType& type, int idx) const
{
    // TODO
    return false;
}

DefaultDeviceInfo Pipewire::getDefaultDevice(const NodeType& type, int idx) const
{
    // TODO
    return {};
}

void Pipewire::setDefaultDevice(const NodeType& type, int idx, const std::string& name, const std::string& dev)
{
    // TODO
}

std::vector<std::string> Pipewire::getDeviceList(const NodeType& type) const
{
    // TODO
    return {};
}

std::list<DefaultDeviceInfo> Pipewire::getSinkList() const
{
    std::list<DefaultDeviceInfo> list;
    uint32_t idx = 0;

    for (const auto* node : m_nodes) {
        if (node->nodeType() != PipewireNode::NodeTypeSink)
            continue;

        DefaultDeviceInfo info;
        info.idx = idx++;
        info.name = node->nodeName();
        info.volume = static_cast<uint32_t>(node->volume() * 100.0f);

        if (std::string::npos != info.name.find("combined")) {
            info.activePortName = "output-combine";
            info.activePortLabel = "Mulit Audio Output";
            info.cardName = "combine-device";
        } else {
            info.cardName = node->nodeName();
            info.activePortName = node->nodeName();
            info.activePortLabel = node->nodeDescription();
        }

        list.push_back(info);
    }

    return list;
}

std::list<DefaultDeviceInfo> Pipewire::getSourceList() const
{
    // TODO
    return {};
}

std::list<std::shared_ptr<DeviceInfo>> Pipewire::getAvailablePortList(const NodeType& type) const
{
    // TODO
    return {};
}

std::list<StreamMediaInfo> Pipewire::getSinkInputList() const
{
    // TODO
    return {};
}

std::list<StreamMediaInfo> Pipewire::getSourceOutputList() const
{
    // TODO
    return {};
}

std::list<StreamMediaIntegrationInfo> Pipewire::getStreamMediaList() const
{
    // TODO
    return {};
}

std::list<std::string> Pipewire::getModuleList()
{
    // TODO
    return {};
}

void Pipewire::setEnabled(std::shared_ptr<RestoreVolumeJson>, const std::string&, const std::string&, bool)
{
    // TODO
}

bool Pipewire::isEnabled(const std::string&, const std::string&) const
{
    // TODO
    return true;
}

bool Pipewire::isLoaded(const ModuleType&) const
{
    // TODO
    return true;
}

bool Pipewire::isValidDevice(const NodeType&) const
{
    // TODO
    return true;
}

// signal
void Pipewire::deviceChanged(const NodeType& type, const std::string& portName, const std::string& cardName)
{
    // TODO
}

void Pipewire::deviceAdjust(const NodeType& type)
{
    // TODO
}

void Pipewire::portChanged(const NodeType& type, const std::string& name)
{
    // TODO
}

void Pipewire::cardRemoved(const std::string& name)
{
    // TODO
}

void Pipewire::addStream(int idx, int value, const std::string& iconName, const std::string& descName)
{
    // TODO
}

void Pipewire::removeStream(int idx)
{
    // TODO
}

void Pipewire::addStream(int dire, int idx, int value, bool mute, const std::string& iconName, const std::string& descName)
{
    // TODO
}

void Pipewire::removeStream(int dire, int idx)
{
    // TODO
}

}
