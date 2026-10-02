#ifndef PULSEAUDIOMANAGER_H
#define PULSEAUDIOMANAGER_H

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/ext-device-restore.h>
#include <pulse/ext-device-manager.h>
#include <glib.h>
#include <memory>
#include <iostream>
#include <optional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <unordered_map>

#include "Util.h"
#include "AudioMethod.h"
#include "AudioContext.h"
#include "AudioManager.h"
#include "PaSinkInputStream.h"
#include "PaSourceOutputStream.h"

namespace UkuiAudioFramwork
{
#define PA_ID_INVALID  ((uint32_t)0xffffffff)

class PulseaudioManager : public AudioManager
{
public:
    PulseaudioManager(int);
    ~PulseaudioManager() = default;

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

    // signal 不需要声明成虚函数只需要内部调用，需要修改
    virtual void deviceChanged(const NodeType&, const std::string&, const std::string&) override;

    virtual void deviceAdjust(const NodeType&) override;

    virtual void portChanged(const NodeType&, const std::string&) override;

    virtual void cardRemoved(const std::string&) override;

    virtual void addStream(int, int, const std::string&, const std::string&) override;

    virtual void removeStream(int) override;

    virtual void addStream(int, int, int, bool, const std::string&, const std::string&) override;

    virtual void removeStream(int, int) override;

    std::shared_ptr<IStream> foundStreamByPortName(const std::string&, const std::string&) const;


private:
    // 模块加载的结果
    enum class ModuleLoadResult {
        Pending,    // 加载未完成
        Succeeded,  // 加载成功
        Failed      // 加载失败
    };

    // 每个模块独立维护加载状态，避免并发加载时互相唤醒
    struct ModuleLoadState {
        std::mutex mutex;
        std::condition_variable condition;
        ModuleLoadResult result = ModuleLoadResult::Pending;
        uint32_t moduleIndex = PA_INVALID_INDEX;
    };

    // 注册模块加载状态；同名模块正在加载时返回空
    std::shared_ptr<ModuleLoadState> registerModuleLoad(const std::string&);

    // 查找指定模块的加载状态
    std::shared_ptr<ModuleLoadState> findModuleLoadState(const std::string&);

    // 保存 PulseAudio 返回的模块索引
    void setModuleLoadIndex(const std::shared_ptr<ModuleLoadState>&, uint32_t);

    // 目标设备属于本次加载模块时，将加载标记为成功
    bool completeModuleLoad(const std::string&, uint32_t);

    // 将未完成的加载标记为失败并唤醒等待线程
    void failModuleLoad(const std::shared_ptr<ModuleLoadState>&);

    // 删除与指定状态匹配的模块加载记录
    void removeModuleLoadState(const std::string&, const std::shared_ptr<ModuleLoadState>&);

    // 取消并清理指定模块的加载状态
    void cancelModuleLoad(const std::string&);

    std::shared_ptr<IStream> foundInfoByName(const NodeType&, const std::string&, const std::string& = "", bool = false) const;

    std::shared_ptr<IStream> foundInfoByName(const NodeType&, int = -1, bool = false) const;

    bool deviceInfosIsEqual(const std::list<std::shared_ptr<DeviceInfo>>&, const std::list<std::shared_ptr<DeviceInfo>>&);

    std::optional<std::shared_ptr<CardInfo>> findCardByName(const std::string&) const;

    std::optional<std::shared_ptr<PaCardPortInfo>> findPortByName(const std::shared_ptr<CardInfo>&, const std::string&) const;

    bool streamIsExitPort(const std::shared_ptr<IStream>&, const std::string&) const;

    std::optional<std::shared_ptr<PaCardProfileInfo2>> findCommonProfile(const std::shared_ptr<PaCardPortInfo>&, const std::shared_ptr<PaCardPortInfo>&) const;

    std::shared_ptr<PaCardProfileInfo2> findHighestPriorityAvailableProfile(const std::vector<std::shared_ptr<PaCardProfileInfo2>>&) const;

    std::optional<std::shared_ptr<PaCardPortInfo>> findHighestPriorityAvailablePort(const std::optional<std::shared_ptr<CardInfo>>&, int, const std::string&) const;

    std::shared_ptr<IStream> findStreamByPort(const std::optional<std::shared_ptr<CardInfo>>&, const std::string&, int) const;

    std::shared_ptr<IStream> findHighestPriorityStream(const std::shared_ptr<IStream>&);

    bool isDefaultDevice(const std::shared_ptr<IStream>&);

    std::vector<std::string> parseSlaves(const std::string&);

    bool isExitEnableDevice(const NodeType&);

    // 当前默认端口不可用或已禁用时，切换到优先级最高的已启用可用端口
    void ensureDefaultOnEnabledDevice(const NodeType& type);

    void resetDefaultSource(const std::string&, const std::shared_ptr<IStream>&);

    void removeStreamMedia(const std::string&, int, int);

    void removeModuleInfoByName(const std::string&);

    std::shared_ptr<IStream> getMasterStream(const IStream* filter, NodeType type) const;

    std::string getBlueMediaTransportPath(GDBusConnection*, const std::string&);

    bool isBluetoothDeviceReady(const std::string&);

    bool evaluateLoadModulePreconditions(const std::string& name, const std::string& param);

    bool shouldShowOsdDedup(const std::string& newDefaultSinkName, long long now);

private:
    static void contextStateCallback(pa_context*, void*);

    static void cardCb(pa_context*, const pa_card_info*, int eol, void*);

    static void simple_callback(pa_context*, int, void*);

    static void sinkCb(pa_context*, const pa_sink_info*, int, void*);

    static void sourceCb(pa_context*, const pa_source_info*, int, void*);

    static void sinkInputCb(pa_context*, const pa_sink_input_info*, int, void*);

    static void sourceOutputCb(pa_context*, const pa_source_output_info*, int, void*);

    static void clientCb(pa_context*, const pa_client_info*, int, void*);

    static void serverInfoCb(pa_context*, const pa_server_info*, void*);

    static void extStreamRestoreReadCb(pa_context*,const pa_ext_stream_restore_info*,int,void*);

    static void extStreamRestoreSubscribeCb(pa_context*, void*);

    static void extDeviceManagerReadCb(pa_context*,const pa_ext_device_manager_info*,int, void*);

    static void extDeviceManagerSubscribeCb(pa_context*, void*);

    static void subscribeCb(pa_context*, pa_subscription_event_type_t, uint32_t, void*);

    static void moduleInfoCb(pa_context*, const pa_module_info*, int, void*);

private:
    int m_reconnectTime;

    bool m_autoPause = false;
    bool m_bContexReady = false;
    bool m_bModuleLoopbackLoading = false;

    // 用于区分“用户手动切换默认输出” vs “设备连接导致的自动切换”
    std::atomic<long long> m_lastUserSetDefaultOutputMs {0};

    // OSD去重：避免同一次默认输出变化弹两次
    std::atomic<long long> m_lastOsdMs {0};
    std::string m_lastOsdDefaultSinkName {};
    mutable std::mutex m_osdMtx;

    mutable std::mutex m_contextMtx;
    // 保护模块加载状态表。
    mutable std::mutex m_moduleStatesMtx;

    mutable std::condition_variable m_contextCond;
    // 按模块名保存相互独立的加载状态。
    std::unordered_map<std::string, std::shared_ptr<ModuleLoadState>> m_moduleStates;

    std::list<StreamMediaIntegrationInfo> m_streamMediaList {};

    static pa_context* m_pContext;
    static pa_mainloop_api* m_pApi;
    PaDataInfo m_info;

    std::unordered_map<std::string, std::string> m_adapterStreamNameMap = {
        {"qaxbrowser", "qaxbrowser-safe"}
    };
};

template <typename DataType, typename GetDataListFunc, typename IsSameDataFunc>
bool updateData(
    PaDataInfo& info,
    const DataType& data,
    GetDataListFunc getDataList,
    IsSameDataFunc isSameData) {

    bool isNew = false;
    auto listOpt = getDataList(info);

    // 检查返回值是否有效
    if (!listOpt.has_value()) {
        AUDIO_ERROR("List is empty or invalid.");
        return isNew;
    }

    auto& list = listOpt.value().get();

    auto it = std::find_if(list.begin(), list.end(),
                           [&data, &isSameData](const DataType& existingData) {
        return isSameData(existingData, data);
    });

    // 如果存在相同的数据则替换，不存在则添加
    if (it != list.end()) {
        // 只更新CardInfo中的card成员变量
        if constexpr (std::is_same_v<std::decay_t<DataType>, std::shared_ptr<CardInfo>>) {
            (*it)->card = data->card;
        }
        else {
            *it = data;
        }
    } else {
        isNew = true;
        list.push_back(data);
    }
    return isNew;
}
}
#endif // PULSEAUDIOMANAGER_H
