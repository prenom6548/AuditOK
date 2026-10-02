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

#include "DbusServer.h"
#include "AudioMethod.h"
#include "AudioContext.h"
#include "PaDataType.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <system_error>


#include <chrono>
#include <optional>
#include <mutex>
#include <unordered_map>

namespace UkuiAudioFramwork
{
const char* AudioDBusServer::m_pIntrospectionXml = R"(
<node>
  <interface name="org.ukui.volume.control">
    <method name="getBackend">
      <arg type="s" name="backend" direction="out"/>
    </method>
    <method name="setBackend">
      <arg type="i" name="backend" direction="in"/>
    </method>
    <method name="getDesktopEnvironmentVersion">
      <arg type="i" name="version" direction="out"/>
    </method>
    <method name="loadModule">
      <arg type="s" name="name" direction="in"/>
      <arg type="s" name="param" direction="in"/>
    </method>
    <method name="unloadModule">
      <arg type="s" name="name" direction="in"/>
    </method>
    <method name="getDetailVolume">
      <arg type="i" name="type" direction="in"/>
      <arg type="s" name="name" direction="in"/>
      <arg type="s" name="card" direction="in"/>
      <arg type="i" name="volume" direction="out"/>
    </method>
    <method name="setDetailVolume">
      <arg type="i" name="type" direction="in"/>
      <arg type="s" name="name" direction="in"/>
      <arg type="s" name="card" direction="in"/>
      <arg type="i" name="volume" direction="in"/>
    </method>
    <method name="getVolume">
      <arg type="i" name="type" direction="in"/>
      <arg type="i" name="index" direction="in"/>
      <arg type="i" name="volume" direction="out"/>
    </method>
    <method name="setVolume">
      <arg type="i" name="type" direction="in"/>
      <arg type="i" name="index" direction="in"/>
      <arg type="i" name="volume" direction="in"/>
    </method>
    <method name="getBalance">
      <arg type="i" name="type" direction="in"/>
      <arg type="d" name="volume" direction="out"/>
    </method>
    <method name="setBalance">
      <arg type="i" name="type" direction="in"/>
      <arg type="d" name="volume" direction="in"/>
    </method>
    <method name="getDetailMute">
      <arg type="i" name="type" direction="in"/>
      <arg type="s" name="name" direction="in"/>
      <arg type="s" name="card" direction="in"/>
      <arg type="b" name="mute" direction="out"/>
    </method>
    <method name="setDetailMute">
      <arg type="i" name="type" direction="in"/>
      <arg type="s" name="name" direction="in"/>
      <arg type="s" name="card" direction="in"/>
      <arg type="b" name="mute" direction="in"/>
    </method>
    <method name="getMute">
      <arg type="i" name="type" direction="in"/>
      <arg type="i" name="index" direction="in"/>
      <arg type="b" name="mute" direction="out"/>
    </method>
    <method name="setMute">
      <arg type="i" name="type" direction="in"/>
      <arg type="i" name="index" direction="in"/>
      <arg type="b" name="mute" direction="in"/>
    </method>
    <method name="getDefaultDevice">
      <arg type="i" name="type" direction="in"/>
      <arg type="i" name="index" direction="in"/>
      <arg type='(issss)' name='default' direction="out"/>
    </method>
    <method name="setDefaultDevice">
      <arg type="i" name="type" direction="in"/>
      <arg type="i" name="index" direction="in"/>
      <arg type="s" name="port" direction="in"/>
      <arg type="s" name="card" direction="in"/>
    </method>
    <method name="getDeviceList">
      <arg type="i" name="type" direction="in"/>
      <arg type="s" name="devList" direction="out"/>
    </method>
    <method name="getSinkList">
      <arg type='a(issss)' name="list" direction="out"/>
    </method>
    <method name="getSourceList">
      <arg type='a(issss)' name="list" direction="out"/>
    </method>
    <method name="getAvailablePortList">
      <arg type="i" name="type" direction="in"/>
      <arg type='a(ssssiiib)' name="portList" direction="out"/>
    </method>
    <method name="getSinkInputList">
      <arg type='a(siiss)' name="list" direction="out"/>
    </method>
    <method name="getSourceOutputList">
      <arg type='a(siiss)' name="list" direction="out"/>
    </method>
    <method name="getStreamMediaList">
      <arg type='a(sssa(iiibs))' name="list" direction="out"/>
    </method>
    <method name="setEnabled">
      <arg type="s" name="card" direction="in"/>
      <arg type="s" name="port" direction="in"/>
      <arg type="b" name="enabled" direction="in"/>
    </method>
    <method name="isEnabled">
      <arg type="s" name="card" direction="in"/>
      <arg type="s" name="port" direction="in"/>
      <arg type="b" name="enabled" direction="out"/>
    </method>
    <method name="setAutoPauseStatus">
      <arg type="b" name="status" direction="in"/>
    </method>
    <method name="getAutoPauseStatus">
      <arg type="b" name="enabled" direction="out"/>
    </method>
    <method name="setMultiAudioCombine">
      <arg type='i' name="type" direction="in"/>
      <arg type='b' name="status" direction="in"/>
      <arg type='as' name="devList" direction="in"/>
    </method>
    <method name="getMultiAudioCombineStatus">
      <arg type='i' name="type" direction="in"/>
      <arg type="b" name="enabled" direction="out"/>
    </method>
    <method name="isValidDevice">
      <arg type='i' name="type" direction="in"/>
      <arg type="b" name="isValid" direction="out"/>
    </method>
    <signal name="volumeChanged">
      <arg type="i" name="type"/>
      <arg type="i" name="index"/>
      <arg type="v" name="volume"/>
    </signal>
    <signal name="muteChanged">
      <arg type="i" name="type"/>
      <arg type="i" name="index"/>
      <arg type="b" name="mute"/>
    </signal>
    <signal name="deviceChanged">
      <arg type="i" name="type"/>
      <arg type="s" name="portName"/>
      <arg type="s" name="cardName"/>
    </signal>
    <signal name="deviceAdjust">
      <arg type="i" name="type"/>
    </signal>
    <signal name="portChanged">
      <arg type="i" name="type"/>
      <arg type="i" name="name"/>
    </signal>
    <signal name="cardRemoved">
      <arg type="s" name="name"/>
    </signal>
    <signal name="addStream">
      <arg type="i" name="index"/>
      <arg type="i" name="value"/>
      <arg type="s" name="iconName"/>
      <arg type="s" name="descName"/>
    </signal>
    <signal name="removeStream">
      <arg type="i" name="type"/>
    </signal>
    <signal name="addStream">
      <arg type="i" name="direction"/>
      <arg type="i" name="index"/>
      <arg type="i" name="value"/>
      <arg type="s" name="iconName"/>
      <arg type="s" name="descName"/>
    </signal>
    <signal name="removeStream">
      <arg type="i" name="direction"/>
      <arg type="i" name="index"/>
    </signal>
  </interface>
</node>
)";

const char* AudioDBusServer::m_pAudioSettingXml = R"(
<node>
  <interface name="org.ukui.audio.settings">
    <method name="getVolumeBoostStatus">
      <arg type="b" name="status" direction="out"/>
    </method>
    <method name="setVolumeBoostStatus">
      <arg type="b" name="status" direction="in"/>
    </method>
    <method name="getVolumeBoostValue">
      <arg type="i" name="value" direction="out"/>
    </method>
    <method name="setVolumeBoostValue">
      <arg type="i" name="value" direction="in"/>
    </method>
    <method name="getMonoStatus">
      <arg type="b" name="status" direction="out"/>
    </method>
    <method name="setMonoStatus">
      <arg type="b" name="status" direction="in"/>
    </method>
    <method name="getEchoCancelStatus">
      <arg type="b" name="status" direction="out"/>
    </method>
    <method name="setEchoCancelStatus">
      <arg type="b" name="status" direction="in"/>
    </method>
    <method name="getLoopbackStatus">
      <arg type="b" name="value" direction="out"/>
    </method>
    <method name="setLoopbackStatus">
      <arg type="b" name="value" direction="in"/>
    </method>
    <method name="getAlertStatus">
      <arg type="b" name="status" direction="out"/>
    </method>
    <method name="setAlertStatus">
      <arg type="b" name="status" direction="in"/>
    </method>
    <method name="getStartupStatus">
      <arg type="b" name="status" direction="out"/>
    </method>
    <method name="setStartupStatus">
      <arg type="b" name="status" direction="in"/>
    </method>
    <method name="getPoweroffStatus">
      <arg type="b" name="status" direction="out"/>
    </method>
    <method name="setPoweroffStatus">
      <arg type="b" name="status" direction="in"/>
    </method>
    <method name="getLogoutStatus">
      <arg type="b" name="status" direction="out"/>
    </method>
    <method name="setLogoutStatus">
      <arg type="b" name="status" direction="in"/>
    </method>
    <method name="getWakeupStatus">
      <arg type="b" name="status" direction="out"/>
    </method>
    <method name="setWakeupStatus">
      <arg type="b" name="status" direction="in"/>
    </method>
    <method name="getGlobalThemeName">
      <arg type="s" name="name" direction="out"/>
    </method>
    <method name="setGlobalThemeName">
      <arg type="s" name="name" direction="in"/>
    </method>
    <method name="getSoundThemeName">
      <arg type="s" name="name" direction="out"/>
    </method>
    <method name="setSoundThemeName">
      <arg type="s" name="name" direction="in"/>
    </method>
    <method name="getCustomThemeStatus">
      <arg type="b" name="status" direction="out"/>
    </method>
    <method name="setCustomThemeStatus">
      <arg type="b" name="status" direction="in"/>
    </method>
    <method name="getNotifyGeneralName">
      <arg type="s" name="name" direction="out"/>
    </method>
    <method name="setNotifyGeneralName">
      <arg type="s" name="name" direction="in"/>
    </method>
    <method name="getVolumeChangedName">
      <arg type="s" name="name" direction="out"/>
    </method>
    <method name="setVolumeChangedName">
      <arg type="s" name="name" direction="in"/>
    </method>
    <signal name="changed">
      <arg type="s" name="key"/>
      <arg type="v" name="value"/>
    </signal>
  </interface>
</node>
)";

const char* AudioDBusServer::m_pSoundThemeXml = R"(
<node>
  <interface name="org.ukui.soundTheme">
    <method name="getSoundThemeList">
      <arg type='a(ss)' name='list' direction='out'/>
    </method>
    <method name="getSoundEffectFileList">
      <arg type='s' name='name' direction='in'/>
      <arg type='a(ss)' name='list' direction='out'/>
    </method>
  </interface>
</node>
)";

const char* AudioDBusServer::m_pSoundThemePlayerXml = R"(
<node>
  <interface name="org.ukui.sound.theme.player">
    <method name="playAlertSound">
      <arg type='b' name='result' direction='out'/>
      <arg type='s' name='soundEvent' direction='in'/>
    </method>
  </interface>
</node>
)";

GDBusInterfaceVTable AudioDBusServer::m_interfaceVTable = {
    &AudioDBusServer::handleMethodCall,
    nullptr,
    nullptr
};

/**
 * D-Bus 对外设置音量入口的合法范围：
 * - OUTPUT(sink)：未开超音量为 [0, 100]；开启超音量为 [0, getVolumeBoostValue()]
 * - INPUT / SINK_INPUT / SOURCE_OUTPUT：固定 [0, 100]
 * 越界值钳位后继续下发，避免非法音量触发 PulseAudio pa_operation_unref(NULL) 崩溃。
 */
static int clampDbusVolume(int type, int volume)
{
    const int minVolume = 0;
    int maxVolume = static_cast<int>(UKMEDIA_VOLUME_NORMAL);
    const auto nodeType = static_cast<NodeType>(type);

    if (nodeType == NodeType::OUTPUT && AudioMethod::getInstance().getVolumeBoostStatus()) {
        maxVolume = AudioMethod::getInstance().getVolumeBoostValue();
        if (maxVolume < static_cast<int>(UKMEDIA_VOLUME_NORMAL)) {
            maxVolume = static_cast<int>(UKMEDIA_VOLUME_NORMAL);
        }
    }

    const int clamped = std::clamp(volume, minVolume, maxVolume);
    if (clamped != volume) {
        AUDIO_WARN("D-Bus volume out of range, type: {}, volume: {}, clamp to [{}, {}] -> {}.",
                   type, volume, minVolume, maxVolume, clamped);
    }
    return clamped;
}

namespace {

/**
 * D-Bus setter 方法的尾沿防抖窗口。
 * 语义：同一个方法在 400ms 内被调用多次时，只在窗口结束后执行最后一次调用参数。
 */
constexpr auto METHOD_DEBOUNCE_INTERVAL = std::chrono::milliseconds(400);

/**
 * GLib 主循环的 source id 是进程内状态，D-Bus 回调和测试清理可能来自不同线程。
 * 所有待执行 source id 的读写都必须通过 g_methodDebounceMutex 保护。
 */
std::mutex g_methodDebounceMutex;
std::unordered_map<std::string, guint> g_debouncedMethodSourceIds;

/**
 * 尚处于防抖窗口、未写入音频后端的 setVolume 参数。
 * sourceId 用于区分先后两次定时任务，避免旧任务清除新任务缓存。
 */
struct PendingVolume {
    int type = 0;
    int index = 0;
    int value = 0;
    guint sourceId = 0;
};

/**
 * 最新一次待执行的音量设置；不存在待执行设置时为空。
 * 与防抖任务共用 g_methodDebounceMutex，保证 D-Bus 查询和定时回调并发安全。
 */
std::optional<PendingVolume> g_pendingVolume;

struct DebouncedVoidMethodCall {
    std::string methodName;

    /**
     * parameters 来自 GDBus 回调。调用入队后原回调会立即返回，所以这里必须持有引用，
     * 等延迟执行或取消时再释放，避免使用已经失效的 GVariant。
     */
    GVariant* parameters = nullptr;
    guint sourceId = 0;
};

/**
 * 真正执行被防抖的 void/setter 方法。
 * Getter 方法不走这里：读操作需要立即返回当前状态，延迟或合并会让调用方拿不到结果。
 */
void executeDebouncedVoidMethod(const std::string& methodName, GVariant* parameters)
{
    if (methodName == "setDetailVolume") {
        int type = 0;
        int volume = 0;
        const gchar* name = nullptr;
        const gchar* dev = nullptr;
        g_variant_get(parameters, "(issi)", &type, &name, &dev, &volume);
        volume = clampDbusVolume(type, volume);
        AudioMethod::getInstance().setDetailVolume(static_cast<NodeType>(type), name, dev, volume);
    } else if (methodName == "setVolume") {
        int type = 0;
        int idx = 0;
        int volume = 0;
        g_variant_get(parameters, "(iii)", &type, &idx, &volume);
        volume = clampDbusVolume(type, volume);
        AudioMethod::getInstance().setVolume(static_cast<NodeType>(type), idx, volume);
    } else if (methodName == "setBalance") {
        int type = 0;
        double volume = 0;
        g_variant_get(parameters, "(id)", &type, &volume);
        AudioMethod::getInstance().setBalance(static_cast<NodeType>(type), volume);
    } else if (methodName == "setDetailMute") {
        int type = 0;
        gboolean mute = FALSE;
        const gchar* name = nullptr;
        const gchar* dev = nullptr;
        g_variant_get(parameters, "(issb)", &type, &name, &dev, &mute);
        AudioMethod::getInstance().setDetailMute(IntToEnum<NodeType>(type), name, dev, mute);
    } else if (methodName == "setMute") {
        int type = 0;
        int idx = 0;
        gboolean mute = FALSE;
        g_variant_get(parameters, "(iib)", &type, &idx, &mute);
        AudioMethod::getInstance().setMute(IntToEnum<NodeType>(type), idx, mute);
    } else if (methodName == "setDefaultDevice") {
        int type = 0;
        int index = 0;
        const gchar* name = nullptr;
        const gchar* dev = nullptr;
        g_variant_get(parameters, "(iiss)", &type, &index, &name, &dev);
        AudioMethod::getInstance().setDefaultDevice(IntToEnum<NodeType>(type), index, name, dev);
    }
}

gboolean runDebouncedVoidMethod(gpointer userData)
{
    auto* call = static_cast<DebouncedVoidMethodCall*>(userData);
    {
        std::lock_guard<std::mutex> guard(g_methodDebounceMutex);
        auto it = g_debouncedMethodSourceIds.find(call->methodName);
        /**
         * 只有当前 source id 仍然是该方法的最新任务时才清理 map。
         * 如果窗口内又来了同方法调用，旧 source 会被 remove，新的 source id 会覆盖 map。
         */
        if (it != g_debouncedMethodSourceIds.end() && it->second == call->sourceId) {
            g_debouncedMethodSourceIds.erase(it);
        }
    }

    executeDebouncedVoidMethod(call->methodName, call->parameters);

    if (call->methodName == "setVolume") {
        std::lock_guard<std::mutex> guard(g_methodDebounceMutex);
        /**
         * 仅清除当前任务对应的缓存。若执行期间已有新的 setVolume 入队，
         * 新缓存的 sourceId 不同，必须保留给后续 getVolume 查询。
         */
        if (g_pendingVolume && g_pendingVolume->sourceId == call->sourceId) {
            g_pendingVolume.reset();
        }
    }
    return G_SOURCE_REMOVE;
}

void destroyDebouncedVoidMethodCall(gpointer userData)
{
    auto* call = static_cast<DebouncedVoidMethodCall*>(userData);
    if (call->parameters) {
        g_variant_unref(call->parameters);
    }
    delete call;
}

bool isDebouncedVoidMethod(const char* methodName)
{
    if (!methodName) {
        return false;
    }

    /**
     * 只对无返回值的 setter/操作类方法做尾沿防抖。
     * 有返回值的方法必须同步返回结果，不能只保存最后一次参数后异步执行。
     */
    static const std::unordered_map<std::string, bool> debouncedMethods = {
        {"setDetailVolume", true},
        {"setVolume", true},
        {"setBalance", true},
        {"setDetailMute", true},
        {"setMute", true},
        {"setDefaultDevice", true},
    };

    return debouncedMethods.find(methodName) != debouncedMethods.end();
}

void enqueueDebouncedVoidMethodCall(const char* methodName, GVariant* parameters)
{
    guint oldSourceId = 0;
    {
        std::lock_guard<std::mutex> guard(g_methodDebounceMutex);
        const auto it = g_debouncedMethodSourceIds.find(methodName);
        if (it != g_debouncedMethodSourceIds.end()) {
            /**
             * 同方法已有等待执行的任务，先取出旧 source id。
             * 锁外调用 g_source_remove，避免 GLib 在销毁回调中间接造成锁重入。
             */
            oldSourceId = it->second;
            g_debouncedMethodSourceIds.erase(it);
        }
    }

    if (oldSourceId != 0) {
        g_source_remove(oldSourceId);
    }

    auto* call = new DebouncedVoidMethodCall{methodName, g_variant_ref(parameters), 0};

    /**
     * 每次新调用都重新启动 200ms 定时器；因此窗口内最后一次调用才会真正执行。
     */
    const guint sourceId = g_timeout_add_full(G_PRIORITY_DEFAULT,
                                              static_cast<guint>(METHOD_DEBOUNCE_INTERVAL.count()),
                                              runDebouncedVoidMethod,
                                              call,
                                              destroyDebouncedVoidMethodCall);
    call->sourceId = sourceId;

    std::lock_guard<std::mutex> guard(g_methodDebounceMutex);
    g_debouncedMethodSourceIds[methodName] = sourceId;
    if (g_strcmp0(methodName, "setVolume") == 0) {
        /**
         * 音量尚未真正写入后端时先缓存最后一次参数，使同步 getVolume
         * 在防抖窗口内也能观察到调用方刚刚设置的值。
         * 缓存前先钳位，避免查询侧读到越界音量。
         */
        PendingVolume pendingVolume;
        g_variant_get(parameters, "(iii)", &pendingVolume.type, &pendingVolume.index, &pendingVolume.value);
        pendingVolume.value = clampDbusVolume(pendingVolume.type, pendingVolume.value);
        pendingVolume.sourceId = sourceId;
        g_pendingVolume = pendingVolume;
    }
}

std::optional<int> getPendingVolume(int type, int index)
{
    std::lock_guard<std::mutex> guard(g_methodDebounceMutex);
    // 只返回同一节点类型和索引的待处理值，避免影响其他音频节点的查询。
    if (g_pendingVolume && g_pendingVolume->type == type && g_pendingVolume->index == index) {
        return g_pendingVolume->value;
    }
    return std::nullopt;
}

bool handleDebouncedVoidMethodCall(const char* methodName, GVariant* parameters, GDBusMethodInvocation* invocation)
{
    if (!isDebouncedVoidMethod(methodName)) {
        return false;
    }

    /**
     * D-Bus 层立即返回成功，避免同步客户端被阻塞。
     * 实际 AudioMethod 调用由 GLib 定时器延后执行，窗口内只保留最后一组参数。
     */
    enqueueDebouncedVoidMethodCall(methodName, parameters);
    g_dbus_method_invocation_return_value(invocation, nullptr);
    return true;
}

void resetMethodDebounceState()
{
    std::vector<guint> sourceIds;
    {
        std::lock_guard<std::mutex> guard(g_methodDebounceMutex);
        for (const auto& [methodName, sourceId] : g_debouncedMethodSourceIds) {
            (void)methodName;
            sourceIds.push_back(sourceId);
        }
        g_debouncedMethodSourceIds.clear();
        g_pendingVolume.reset();
    }

    /**
     * 先拷贝 source id，再在锁外移除。g_source_remove 可能触发 destroy notify，
     * destroy notify 会释放 DebouncedVoidMethodCall 和其持有的 GVariant 引用。
     */
    for (const guint sourceId : sourceIds) {
        g_source_remove(sourceId);
    }

    std::lock_guard<std::mutex> guard(g_methodDebounceMutex);
    g_debouncedMethodSourceIds.clear();
}

} // namespace

std::string getProcessName(pid_t pid)
{
    std::ostringstream oss;
    oss << "/proc/" << pid << "/exe";
    std::string procPath = oss.str();
    try {
        std::filesystem::path targetPath = std::filesystem::read_symlink(procPath);
        std::string path = targetPath.string();
        size_t pos = path.rfind('/');
        if (pos != std::string::npos) {
            return path.substr(pos + 1);
        } else {
            return path;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        return "Unknown";
    }
}


bool AudioDBusServer::initialize() {
    GError* error = nullptr;
    struct InitializeGuard {
        AudioDBusServer* self;
        bool succeeded = false;
        ~InitializeGuard()
        {
            if (!succeeded && self->m_pConnection) {
                g_object_unref(self->m_pConnection);
                self->m_pConnection = nullptr;
            }
        }
    } guard{this};

    m_pConnection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
    if (!m_pConnection) {
        AUDIO_ERROR("Failed to connect to DBus: {}.", error ? error->message : "unknown");
        if (error) g_error_free(error);
        return false;
    }

    GDBusProxy* proxy = nullptr;
    proxy = g_dbus_proxy_new_sync(
        m_pConnection,
        G_DBUS_PROXY_FLAGS_NONE,
        nullptr,
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        nullptr,
        &error);
    if (!proxy) {
        AUDIO_ERROR("Failed to create DBus proxy: {}.", error->message);
        g_error_free(error);
        return false;
    }
    GVariant* result = nullptr;
    result = g_dbus_proxy_call_sync(
        proxy,
        "RequestName",
        g_variant_new("(su)", AUDIO_OBJECT_SERVICE, 0),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        &error);
    g_object_unref(proxy);
    if (!result) {
        AUDIO_ERROR("Failed to request service name: {}.", error->message);
        g_error_free(error);
        return false;
    }
    g_variant_unref(result);

    const char* introspectionXml = m_pIntrospectionXml;
    auto introspectionData = g_dbus_node_info_new_for_xml(introspectionXml, &error);
    if (!introspectionData) {
        AUDIO_ERROR("Failed to parse introspection XML: {}.", error->message);
        g_error_free(error);
        return false;
    }

    g_dbus_connection_register_object(
        m_pConnection,
        AUDIO_OBJECT_PATH,
        introspectionData->interfaces[0],
        &m_interfaceVTable,
        this,
        nullptr,
        &error);


    if (error) {
        AUDIO_ERROR("Failed to register object: {}.", error->message);
        g_error_free(error);
        return false;
    }

    const char* audioSettingXml = m_pAudioSettingXml;
    auto audioSettingData = g_dbus_node_info_new_for_xml(audioSettingXml, &error);
    if (!audioSettingData) {
        AUDIO_ERROR("Failed to parse audioSetting XML: {}.", error->message);
        g_error_free(error);
        return false;
    }

    g_dbus_connection_register_object(
        m_pConnection,
        AUDIO_OBJECT_PATH,
        audioSettingData->interfaces[0],
        &m_interfaceVTable,
        this,
        nullptr,
        &error);


    if (error) {
        AUDIO_ERROR("Failed to register object: {}.", error->message);
        g_error_free(error);
        return false;
    }

    const char* soundThemeXml = m_pSoundThemeXml;
    auto soundThemeData = g_dbus_node_info_new_for_xml(soundThemeXml, &error);
    if (!soundThemeData) {
        AUDIO_ERROR("Failed to parse audioSetting XML: {}", error->message);
        g_error_free(error);
        return false;
    }

    g_dbus_connection_register_object(
        m_pConnection,
        AUDIO_OBJECT_PATH,
        soundThemeData->interfaces[0],
        &m_interfaceVTable,
        this,
        nullptr,
        &error);


    if (error) {
        AUDIO_ERROR("Failed to register object: {}.", error->message);
        g_error_free(error);
        return false;
    }

    // Register sound theme player interface
    auto soundThemePlayerData = g_dbus_node_info_new_for_xml(m_pSoundThemePlayerXml, &error);
    if (!soundThemePlayerData) {
        AUDIO_ERROR("Failed to parse soundThemePlayer XML: {}", error->message);
        g_error_free(error);
        return false;
    }

    g_dbus_connection_register_object(
        m_pConnection,
        "/org/ukui/sound/theme/player",  // 使用原来的对象路径
        soundThemePlayerData->interfaces[0],
        &m_interfaceVTable,
        this,
        nullptr,
        &error);

    if (error) {
        AUDIO_ERROR("Failed to register sound theme player object: {}.", error->message);
        g_error_free(error);
        return false;
    }

    // 请求原来的 Bus Name，保持兼容性
    auto nameProxy = g_dbus_proxy_new_sync(
        m_pConnection,
        G_DBUS_PROXY_FLAGS_NONE,
        nullptr,
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        nullptr,
        &error);
    if (nameProxy) {
        auto nameResult = g_dbus_proxy_call_sync(
            nameProxy,
            "RequestName",
            g_variant_new("(su)", "org.ukui.sound.theme.player", 0),  // 使用原来的 Bus Name
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            &error);
        if (nameResult) {
            AUDIO_INFO("Successfully registered D-Bus service: org.ukui.sound.theme.player");
            g_variant_unref(nameResult);
        } else {
            AUDIO_ERROR("Failed to request service name org.ukui.sound.theme.player: {}", error ? error->message : "unknown");
            if (error) g_error_free(error);
            error = nullptr;
        }
        g_object_unref(nameProxy);
    } else {
        AUDIO_ERROR("Failed to create DBus proxy for name request: {}", error ? error->message : "unknown");
        if (error) g_error_free(error);
    }

    m_pMainLoop = g_main_loop_new(nullptr, false);
    guard.succeeded = true;
    return true;
}

void AudioDBusServer::run() {
    g_main_loop_run(m_pMainLoop);
}


void AudioDBusServer::sendSignal(const char* objectPath, const char* interfaceName, const char* signalName, GVariant* param)
{
    g_dbus_connection_emit_signal(
        m_pConnection,
        nullptr,
        objectPath,
        interfaceName,
        signalName,
        param,
        nullptr);
}

void AudioDBusServer::sendSignal(const char* busName, const char* objectPath, const char* interfaceName, const char* methodName, GVariant* param)
{
    GError *err = nullptr;

    GVariant *result = g_dbus_connection_call_sync(m_pConnection,
                                                   busName,
                                                   objectPath,
                                                   interfaceName,
                                                   methodName,
                                                   param,
                                                   nullptr,
                                                   G_DBUS_CALL_FLAGS_NONE,
                                                   -1,
                                                   nullptr,
                                                   &err);
    if (result) {
        g_variant_unref(result);
    }

    if (err) {
        AUDIO_ERROR("Unable to get the caller process, error: {}", err->message);
        g_error_free(err);
    } else {
        AUDIO_DEBUG("call the {} method suc.", methodName);
    }
}

void AudioDBusServer::handleMethodCall(GDBusConnection* connection,
                             const char* sender,
                             const char* objectPath,
                             const char* interfaceName,
                             const char* methodName,
                             GVariant* parameters,
                             GDBusMethodInvocation* invocation,
                             gpointer userData) {
    AudioDBusServer* self = static_cast<AudioDBusServer*>(userData);

    GError *err = nullptr;
    guint pid = 0;

    GVariant *result = g_dbus_connection_call_sync(connection,
                                                   "org.freedesktop.DBus",
                                                   "/org/freedesktop/DBus",
                                                   "org.freedesktop.DBus",
                                                   "GetConnectionUnixProcessID",
                                                   g_variant_new("(s)", sender),
                                                   G_VARIANT_TYPE("(u)"),
                                                   G_DBUS_CALL_FLAGS_NONE,
                                                   -1,
                                                   nullptr,
                                                   &err);
    if (result) {
        g_variant_get(result, "(u)", &pid);
        g_variant_unref(result);
    }

    if (err) {
        AUDIO_ERROR("Unable to get the caller process, error: {}", err->message);
        g_error_free(err);
    } else {
        AUDIO_DEBUG("{} calling the {} method.", getProcessName(pid), methodName);
    }

    /**
     * 防抖 setter 必须在普通分发链之前处理，否则第一次调用会立即生效，
     * 无法实现“窗口内只执行最后一次设置”的语义。
     */
    if (handleDebouncedVoidMethodCall(methodName, parameters, invocation)) {
        AUDIO_DEBUG("{} method call is queued for trailing debounce.", methodName);
        return;
    }

    if (g_strcmp0(methodName, "getBackend") == 0) {
        std::string backend = AudioMethod::getInstance().getBackend();
        g_dbus_method_invocation_return_value(invocation,
                                              g_variant_new("(s)", backend.c_str()));
    }
    else if (g_strcmp0(methodName, "setBackend") == 0) {
        uint32_t type;

        g_variant_get(parameters, "(i)", &type);
        AudioMethod::getInstance().setBackend(IntToEnum<BackendType>(type));
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getDesktopEnvironmentVersion") == 0) {
        auto version = AudioMethod::getInstance().getDesktopEnvironmentVersion();

        g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", version));
    }
    else if (g_strcmp0(methodName, "loadModule") == 0) {
        const gchar* name, *param;

        g_variant_get(parameters, "(ss)", &name, &param);
        AudioMethod::getInstance().loadModule(name, param);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "unloadModule") == 0) {
        const gchar* name;

        g_variant_get(parameters, "(s)", &name);
        AudioMethod::getInstance().unloadModule(name);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getDetailVolume") == 0) {
        uint32_t type;
        const gchar* name;
        const gchar* dev;

        g_variant_get(parameters, "(iss)", &type, &name, &dev);
        auto volume = AudioMethod::getInstance().getDetailVolume(IntToEnum<NodeType>(type), name, dev);
        g_dbus_method_invocation_return_value(invocation,
                                               g_variant_new("(i)", volume));
    }
    else if (g_strcmp0(methodName, "setDetailVolume") == 0) {
        int type = 0;
        int volume = 0;
        const gchar* name;
        const gchar* dev;

        g_variant_get(parameters, "(issi)", &type, &name, &dev, &volume);
        volume = clampDbusVolume(type, volume);
        AudioMethod::getInstance().setDetailVolume(static_cast<NodeType>(type), name, dev, volume);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getVolume") == 0) {
        int type = 0;
        int idx = 0;

        g_variant_get(parameters, "(ii)", &type, &idx);
        // 防抖期间后端仍是旧值，因此优先返回尚未执行的最新 setVolume 参数。
        const auto pendingVolume = getPendingVolume(type, idx);
        const auto volume = pendingVolume
            ? *pendingVolume
            : AudioMethod::getInstance().getVolume(IntToEnum<NodeType>(type), idx);
        g_dbus_method_invocation_return_value(invocation,
                                               g_variant_new("(i)", volume));
    }
    else if (g_strcmp0(methodName, "setVolume") == 0) {
        int type = 0;
        int idx = 0;
        int volume = 0;

        g_variant_get(parameters, "(iii)", &type, &idx, &volume);
        volume = clampDbusVolume(type, volume);
        AudioMethod::getInstance().setVolume(static_cast<NodeType>(type), idx, volume);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }

    else if (g_strcmp0(methodName, "getBalance") == 0) {
        uint32_t type;

        g_variant_get(parameters, "(i)", &type);
        auto volume = AudioMethod::getInstance().getBalance(IntToEnum<NodeType>(type));
        g_dbus_method_invocation_return_value(invocation,
                                              g_variant_new("(d)", volume));
    }
    else if (g_strcmp0(methodName, "setBalance") == 0) {
        uint32_t type;
        double volume;

        g_variant_get(parameters, "(id)", &type, &volume);
        AudioMethod::getInstance().setBalance(static_cast<NodeType>(type), volume);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getDetailMute") == 0) {
        uint32_t type;
        const gchar* name;
        const gchar* dev;

        g_variant_get(parameters, "(iss)", &type, &name, &dev);
        auto mute = AudioMethod::getInstance().getDetailMute(IntToEnum<NodeType>(type), name, dev);
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", mute));
    }
    else if (g_strcmp0(methodName, "setDetailMute") == 0) {
        uint32_t type;
        bool mute = false;
        const gchar* name;
        const gchar* dev;

        g_variant_get(parameters, "(issb)", &type, &name, &dev, &mute);
        AudioMethod::getInstance().setDetailMute(IntToEnum<NodeType>(type), name, dev, mute);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getMute") == 0) {
        uint32_t type, idx;

        g_variant_get(parameters, "(ii)", &type, &idx);
        auto mute = AudioMethod::getInstance().getMute(IntToEnum<NodeType>(type), idx);
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", mute));
    }
    else if (g_strcmp0(methodName, "setMute") == 0) {
        uint32_t type, idx;
        bool mute = false;

        g_variant_get(parameters, "(iib)", &type, &idx, &mute);
        AudioMethod::getInstance().setMute(IntToEnum<NodeType>(type), idx, mute);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getDefaultDevice") == 0) {
        uint32_t type, idx;

        g_variant_get(parameters, "(ii)", &type, &idx);
        auto info = AudioMethod::getInstance().getDefaultDevice(IntToEnum<NodeType>(type), idx);

        g_dbus_method_invocation_return_value(invocation, g_variant_new("((issss))", info.idx, info.name.c_str(), info.cardName.c_str(),
                                              info.activePortName.c_str(), info.activePortLabel.c_str()));
    }
    else if (g_strcmp0(methodName, "setDefaultDevice") == 0) {
        uint32_t type, index;
        const gchar* dev;
        const gchar* name;

        g_variant_get(parameters, "(iiss)", &type, &index, &name, &dev);
        AudioMethod::getInstance().setDefaultDevice(IntToEnum<NodeType>(type), index, name, dev);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getDeviceList") == 0) {
        uint32_t type;

        g_variant_get(parameters, "(i)", &type);
//        dev = AudioMethod::getInstance().getDeviceList(IntToEnum<NodeType>(type));
//        g_dbus_method_invocation_return_value(invocation, g_variant_new("(as)", dev));
    }
    else if (g_strcmp0(methodName, "getSinkList") == 0) {
        auto list = AudioMethod::getInstance().getSinkList();

        GVariantBuilder ret;
        g_variant_builder_init(&ret, G_VARIANT_TYPE("a(issss)"));
        for (const auto& s : list) {
            g_variant_builder_add(&ret, "(issss)", s.idx, s.name.c_str(), s.cardName.c_str(),
                                  s.activePortName.c_str(), s.activePortLabel.c_str());
        }
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(a(issss))", &ret));
    }
    else if (g_strcmp0(methodName, "getSourceList") == 0) {
        auto list = AudioMethod::getInstance().getSourceList();

        GVariantBuilder ret;
        g_variant_builder_init(&ret, G_VARIANT_TYPE("a(issss)"));
        for (const auto& s : list) {
            g_variant_builder_add(&ret, "(issss)", s.idx, s.name.c_str(), s.cardName.c_str(), s.activePortName.c_str(),
                                  s.activePortLabel.c_str());
        }
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(a(issss))", &ret));
    }
    else if (g_strcmp0(methodName, "getAvailablePortList") == 0) {
        uint32_t type;
        g_variant_get(parameters, "(i)", &type);

        auto devList = AudioMethod::getInstance().getAvailablePortList(IntToEnum<NodeType>(type));

        GVariantBuilder ret;
        g_variant_builder_init(&ret, G_VARIANT_TYPE("a(ssssiiib)"));
        for (const auto& d : devList) {
            g_variant_builder_add(&ret, "(ssssiiib)", d->cardName.c_str(), d->cardDesc.c_str(), d->portName.c_str(), d->portLabel.c_str(),
                                  d->priority, d->direction, d->available, d->enabled);

        }
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(a(ssssiiib))", &ret));
    }
    else if (g_strcmp0(methodName, "getSinkInputList") == 0) {
        auto siList = AudioMethod::getInstance().getSinkInputList();

        GVariantBuilder ret;
        g_variant_builder_init(&ret, G_VARIANT_TYPE("a(siiss)"));
        for (const auto& si : siList) {
            g_variant_builder_add(&ret, "(siiss)", si.name.c_str(), si.index, si.volume, si.mediaRole.c_str(), si.iconName.c_str());
        }
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(a(siiss))", &ret));
    }
    else if (g_strcmp0(methodName, "getSourceOutputList") == 0) {
        auto soList = AudioMethod::getInstance().getSourceOutputList();

        GVariantBuilder ret;
        g_variant_builder_init(&ret, G_VARIANT_TYPE("a(siiss)"));
        for (const auto& so : soList) {
            g_variant_builder_add(&ret, "(siiss)", so.name.c_str(), so.index, so.volume, so.mediaRole.c_str(), so.iconName.c_str());
        }
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(a(siiss))", &ret));
    }
    else if (g_strcmp0(methodName, "getStreamMediaList") == 0) {
        auto streamList = AudioMethod::getInstance().getStreamMediaList();

        GVariantBuilder ret;
        g_variant_builder_init(&ret, G_VARIANT_TYPE("a(sssa(iiibs))"));

        for (const auto& s : streamList) {
            GVariantBuilder ret1;
            g_variant_builder_init(&ret1, G_VARIANT_TYPE("a(iiibs)"));
            for (const auto& media : s.mediaInfoList) {
                g_variant_builder_add(&ret1, "(iiibs)", media.direction, media.index, media.volume, media.mute, media.name.c_str());
            }
            g_variant_builder_add(&ret, "(sssa(iiibs))", s.mediaRole.c_str(), s.iconName.c_str(), s.binary.c_str(), &ret1);
        }
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(a(sssa(iiibs)))", &ret));
    }
    else if (g_strcmp0(methodName, "setEnabled") == 0) {
        bool enable;
        const gchar* port;
        const gchar* card;

        g_variant_get(parameters, "(ssb)", &card, &port, &enable);
        AudioMethod::getInstance().setEnabled(card, port, enable);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "isEnabled") == 0) {
        const gchar* port;
        const gchar* card;

        g_variant_get(parameters, "(ss)", &card, &port);
        auto status = AudioMethod::getInstance().isEnabled(card, port);
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setAutoPauseStatus") == 0) {
        bool status;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setAutoPauseStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getAutoPauseStatus") == 0) {

        auto status = AudioMethod::getInstance().getAutoPauseStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setMultiAudioCombine") == 0) {
        int type;
        gboolean status;
        const gchar* s;
        GVariantIter* iter;
        std::vector<std::string> devList{};
        g_variant_get(parameters, "(ibas)", &type, &status, &iter);

        while (g_variant_iter_loop (iter, "s", &s))
            devList.push_back(s);
        g_variant_iter_free (iter);

        AudioMethod::getInstance().setMultiAudioCombine(IntToEnum<NodeType>(type), status, devList);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getMultiAudioCombineStatus") == 0) {
        int type;

        g_variant_get(parameters, "(i)", &type);
        auto status = AudioMethod::getInstance().getMultiAudioCombineStatus(IntToEnum<NodeType>(type));
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "isValidDevice") == 0) {
        int type;

        g_variant_get(parameters, "(i)", &type);
        auto status = AudioMethod::getInstance().isValidDevice(IntToEnum<NodeType>(type));

        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "getVolumeBoostStatus") == 0) {

        auto status = AudioMethod::getInstance().getVolumeBoostStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setVolumeBoostStatus") == 0) {
        bool status = false;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setVolumeBoostStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getVolumeBoostValue") == 0) {

        auto value = AudioMethod::getInstance().getVolumeBoostValue();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(i)", value));
    }
    else if (g_strcmp0(methodName, "setVolumeBoostValue") == 0) {
        uint32_t value;

        g_variant_get(parameters, "(i)", &value);
        AudioMethod::getInstance().setVolumeBoostValue(value);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getMonoStatus") == 0) {

        auto status = AudioMethod::getInstance().getMonoStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setMonoStatus") == 0) {
        bool status = false;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setMonoStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getEchoCancelStatus") == 0) {

        auto status = AudioMethod::getInstance().getEchoCancelStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setEchoCancelStatus") == 0) {
        bool status = false;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setEchoCancelStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getLoopbackStatus") == 0) {

        auto status = AudioMethod::getInstance().getLoopbackStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setLoopbackStatus") == 0) {
        bool status = false;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setLoopbackStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getAlertStatus") == 0) {

        auto status = AudioMethod::getInstance().getAlertStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setAlertStatus") == 0) {
        bool status = false;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setAlertStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getStartupStatus") == 0) {

        auto status = AudioMethod::getInstance().getStartupStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setStartupStatus") == 0) {
        auto status = false;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setStartupStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getPoweroffStatus") == 0) {

        auto status = AudioMethod::getInstance().getPoweroffStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setPoweroffStatus") == 0) {
        auto status = false;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setPoweroffStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getLogoutStatus") == 0) {

        auto status = AudioMethod::getInstance().getLogoutStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setLogoutStatus") == 0) {
        bool status = false;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setLogoutStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getWakeupStatus") == 0) {

        auto status = AudioMethod::getInstance().getWakeupStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setWakeupStatus") == 0) {
        auto status = false;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setWakeupStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getGlobalThemeName") == 0) {

        auto name = AudioMethod::getInstance().getGlobalThemeName();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", name.c_str()));
    }
    else if (g_strcmp0(methodName, "setGlobalThemeName") == 0) {
        std::string name;

        g_variant_get(parameters, "(s)", &name);
        AudioMethod::getInstance().setGlobalThemeName(name);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getSoundThemeName") == 0) {

        auto name = AudioMethod::getInstance().getSoundThemeName();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", name.c_str()));
    }
    else if (g_strcmp0(methodName, "setSoundThemeName") == 0) {
        auto name = "";

        g_variant_get(parameters, "(s)", &name);
        AudioMethod::getInstance().setSoundThemeName(name);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getCustomThemeStatus") == 0) {

        auto status = AudioMethod::getInstance().getCustomThemeStatus();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", status));
    }
    else if (g_strcmp0(methodName, "setCustomThemeStatus") == 0) {
        bool status;

        g_variant_get(parameters, "(b)", &status);
        AudioMethod::getInstance().setCustomThemeStatus(status);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getNotifyGeneralName") == 0) {

        auto name = AudioMethod::getInstance().getNotifyGeneralName();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", name.c_str()));
    }
    else if (g_strcmp0(methodName, "setNotifyGeneralName") == 0) {
        auto name = "";

        g_variant_get(parameters, "(s)", &name);
        AudioMethod::getInstance().setNotifyGeneralName(name);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getVolumeChangedName") == 0) {

        auto name = AudioMethod::getInstance().getVolumeChangedName();
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", name.c_str()));
    }
    else if (g_strcmp0(methodName, "setVolumeChangedName") == 0) {
        auto name = "";

        g_variant_get(parameters, "(s)", &name);
        AudioMethod::getInstance().setVolumeChangedName(name);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    }
    else if (g_strcmp0(methodName, "getSoundThemeList") == 0) {
        auto themeList = AudioMethod::getInstance().getSoundThemeList();

        GVariantBuilder ret;
        g_variant_builder_init(&ret, G_VARIANT_TYPE("a(ss)"));
        for (const auto& m : themeList) {
            g_variant_builder_add(&ret, "(ss)", m.name.c_str(), m.description.c_str());
        }
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(a(ss))", &ret));
    }
    else if (g_strcmp0(methodName, "getSoundEffectFileList") == 0) {
        const gchar* name;

        g_variant_get(parameters, "(s)", &name);
        std::list<SoundEffectFileInfo> fileList = AudioMethod::getInstance().getSoundEffectFileList(name);

        GVariantBuilder ret;
        g_variant_builder_init(&ret, G_VARIANT_TYPE("a(ss)"));
        for (const auto& f : fileList) {
            g_variant_builder_add(&ret, "(ss)", f.name.c_str(), f.description.c_str());
        }
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(a(ss))", &ret));
    }
    else if (g_strcmp0(methodName, "playAlertSound") == 0) {
        const gchar* soundEvent;

        g_variant_get(parameters, "(s)", &soundEvent);
        bool result = AudioMethod::getInstance().playAlertSound(soundEvent);

        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", result));
    }
}


AudioDBusServer::~AudioDBusServer() {
    try {
        resetMethodDebounceState();
    } catch (...) {
        // 析构中禁止异常逃逸
    }
    if (m_pConnection) {
        g_object_unref(m_pConnection);
        m_pConnection = nullptr;
    }
    if (m_pMainLoop) {
        g_main_loop_unref(m_pMainLoop);
        m_pMainLoop = nullptr;
    }
}
}
