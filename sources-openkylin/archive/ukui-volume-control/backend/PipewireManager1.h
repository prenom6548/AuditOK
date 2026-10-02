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

#include <vector>
#include <list>
#include <mutex>
#include <unordered_map>
#include <pipewire/pipewire.h>
#include <pipewire/extensions/metadata.h>

#include "PipewireMetadata.h"
#include "PipewireSettings.h"
#include "PipewireNode.h"
#include "PipewireProfiler.h"
//#include "PipewireNodeListmodel.h"
#include "media-session/alsaproperties.h"
#include "SystemdService.h"
#include "AudioManager.h"

struct pw_impl_module;

namespace UkuiAudioFramwork
{
class PipewireMetadata;
class PipewireSettings;
class PipewireLink;
class PipewirePort;
class PipewireDevice;
class PipewireClient;
class AlsaProperties;

class Pipewire : public AudioManager
{
public:
    virtual void init() override;

    virtual bool connect() override;

    // Interface
    virtual void loadModule(const std::string&, const std::string&) override;

    virtual void unloadModule(const std::string&, bool restoreMasterDevice = true) override;

    // Method
    virtual void setDetailVolume(const NodeType&, const std::string&, const std::string&, int) override;

    virtual int getDetailVolume(const NodeType&, const std::string&, const std::string&) const override;

    virtual void setVolume(const NodeType&, int, int) override;

    virtual int getVolume(const NodeType&, int) const override;

    virtual void setDetailMute(const NodeType&, const std::string&, const std::string&, bool) override;

    virtual bool getDetailMute(const NodeType&, const std::string&, const std::string&) const override;

    virtual void setBalance(const NodeType&, double) override;

    virtual double getBalance(const NodeType&) const override;

    virtual void setMute(const NodeType&, int, bool) override;

    virtual bool getMute(const NodeType&, int) const override;

    virtual DefaultDeviceInfo getDefaultDevice(const NodeType&, int) const override;

    virtual void setDefaultDevice(const NodeType&, int, const std::string&, const std::string&) override;

    virtual std::vector<std::string> getDeviceList(const NodeType&) const override;

    virtual std::list<DefaultDeviceInfo> getSinkList() const override;

    virtual std::list<DefaultDeviceInfo> getSourceList() const override;

    virtual std::list<std::shared_ptr<DeviceInfo>> getAvailablePortList(const NodeType&) const override;

    virtual std::list<StreamMediaInfo> getSinkInputList() const override;

    virtual std::list<StreamMediaInfo> getSourceOutputList() const override;

    virtual std::list<StreamMediaIntegrationInfo> getStreamMediaList() const override;

    virtual std::list<std::string> getModuleList() override;

    virtual void setEnabled(std::shared_ptr<RestoreVolumeJson>, const std::string&, const std::string&, bool) override;

    virtual bool isEnabled(const std::string&, const std::string&) const override;

    virtual bool isLoaded(const ModuleType&) const override;

    virtual bool isValidDevice(const NodeType&) const override;

    // signal
    virtual void deviceChanged(const NodeType&, const std::string&, const std::string&) override;

    virtual void deviceAdjust(const NodeType&) override;

    virtual void portChanged(const NodeType&, const std::string&) override;

    virtual void cardRemoved(const std::string&) override;

    virtual void addStream(int, int, const std::string&, const std::string&) override;

    virtual void removeStream(int) override;

    virtual void addStream(int, int, int, bool, const std::string&, const std::string&) override;

    virtual void removeStream(int, int) override;


//signals:
    void quit();
    void registryObject(uint32_t id,
                        uint32_t permissions,
                        const char *type,
                        uint32_t version,
                        const struct spa_dict *props);

    void appVersionChanged(); //bogus, never emitted
    void settingsChanged();
//    void clientChanged();
    void nodesChanged();
    void linksChanged();
    void portsChanged();
    void profilerChanged();
    void pipewireMediaSessionChanged();
    void wirePlumberServiceChanged();
    void alsaPropertiesChanged();

public:
     struct pw_registry *registry = nullptr;
//     PipewireNodeListModel *m_nodes = nullptr;

     std::vector<PipewireNode*> m_nodes;

private:
    struct pw_main_loop *loop = nullptr;

    struct pw_context *context = nullptr;

    struct pw_core *core = nullptr;
    struct spa_hook core_listener;

//    std::vector<PipewireNode*> m_nodes;

//    struct pw_registry *registry = nullptr;
    struct spa_hook registry_listener;

    int sync = 0;
    bool round_trip_done = false;

    std::vector<PipewireClient*> pw_clients = {};
    PipewireSettings *pw_settings = nullptr;
//    PipewireNodeListModel *m_nodes = nullptr;
    std::vector<PipewireLink*> m_links;
    std::list<PipewirePort*> m_ports;
    std::list<PipewireDevice*> m_devices;
    PipewireProfiler *pw_profiler = nullptr;
    SystemdService *pipewire_media_session = nullptr;
    SystemdService *wireplumber_service = nullptr;
    AlsaProperties *alsa_properties = nullptr;

    std::mutex m_moduleMtx;
    std::unordered_map<std::string, pw_impl_module*> m_loadedModules;

public:
    explicit Pipewire(int *argc, char **argv[]);
    virtual ~Pipewire();

    /**
     * Executes pipewire main loop, calling callbacks and other stuff.
     * Runs in main thread and is blocking until all operations are done.
     */
//    Q_INVOKABLE void round_trip();
    void round_trip();


//    QString appVersion() { return QString(PROJECT_VERSION); }
//    Q_INVOKABLE QString pipewireCompiledVersion() { return pw_get_headers_version(); }
//    Q_INVOKABLE QString pipewireLinkedVersion() { return pw_get_library_version(); }
//    Q_INVOKABLE QString platformName() { return QGuiApplication::platformName(); }
//    Q_INVOKABLE QString qtCompiledVersion() { return QStringLiteral(QT_VERSION_STR); }
//    Q_INVOKABLE QString qtLinkedVersion() { return QString::fromLocal8Bit(qVersion()); }
//    Q_INVOKABLE QString kframeworksCompiledVersion() { return QStringLiteral(KF5_COMPILED_VERSION); }

//    Q_INVOKABLE static QString formatTime(double val);
//    Q_INVOKABLE bool isPipewireMediaSession() {
//        //Wireplumber may mask itself as pipewire-media-session, maybe not but let's be double sure
//        return pipewire_media_session->running() && !wireplumber_service->running();
//    }
//    Q_INVOKABLE bool isWireplumber() {
//        return wireplumber_service->running();
//    }
    static std::string formatTime(double val);
    bool isPipewireMediaSession() {
        //Wireplumber may mask itself as pipewire-media-session, maybe not but let's be double sure
        return pipewire_media_session->running() && !wireplumber_service->running();
    }
    bool isWireplumber() {
        return wireplumber_service->running();
    }


//    QPipewireClient* client() { return pw_client; }
    PipewireSettings* settings() { return pw_settings; }
    PipewireProfiler* profiler() { return pw_profiler; }
//    PipewireNodeListModel* nodes() { return m_nodes; }
//    std::vector<PipewireNode*> nodeList() { return m_nodes->list(); }
    std::vector<PipewireLink*> linkList() { return m_links; }
    std::list<PipewirePort*> portList() { return m_ports; }
    SystemdService* pipewireMediaSession() { return pipewire_media_session; }
    SystemdService* wirePlumberService() { return wireplumber_service; }
    AlsaProperties* alsaProperties() { return alsa_properties; }
//    QObjectList nodeObjectList() {
//        auto list = QObjectList();
//        for(int i=0; i<m_nodes->size(); i++) {
//            list.append(m_nodes->list()[i]);
//        }
//        return list;
//    }

public: // actually, private
    // PIPEWIRES CALLBACKS (private)
    void _quit();
    void _on_core_done(uint32_t id, int seq);
    void _on_core_error(uint32_t id, int seq, int res, const char *message);
    void _registry_event(uint32_t id,
                         uint32_t permissions,
                         const char *type,
                         uint32_t version,
                         const struct spa_dict *props);
    void _registry_event_remove(uint32_t id);

//private:
public:
    void resync();


    friend class QPipewireClient;
    friend class QPipewireMetadata;
    friend class QPipewireProfiler;
	friend class QPipewireNode;
	friend class QPipewireLink;
	friend class QPipewirePort;
    friend class QPipewireDevice;
};
}
