#ifndef DBUSCLIENT_H
#define DBUSCLIENT_H

#include <QDBusError>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDBusMetaType>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QVariant>

// DBus info
#define AUDIO_OBJECT_SERVICE "org.ukui.volume.control"
#define AUDIO_OBJECT_PATH    "/org/ukui/volume/control"
#define AUDIO_INTERFACE      "org.ukui.volume.control"
#define SETTINGS_INTERFACE   "org.ukui.audio.settings"
#define SOUNDTHEME_INTERFACE "org.ukui.soundTheme"

// 注册 ClientDeviceInfo 类型
typedef struct ClientDeviceInfo
{
    QString cardName;  // 声卡名称
    QString cardDesc;  // 声卡描述
    QString portName;  // 设备端口名称
    QString portLabel; // 设备端口描述
    quint32 priority;  // 设备端口优先级
    quint32 direction; // 端口方向 input or output
    quint32 available; // 端口可用状态
    bool enabled;      // 端口启用状态
    friend const QDBusArgument& operator>>(const QDBusArgument& argument, ClientDeviceInfo& info) {
        argument.beginStructure();
        argument >> info.cardName;
        argument >> info.cardDesc;
        argument >> info.portName;
        argument >> info.portLabel;
        argument >> info.priority;
        argument >> info.direction;
        argument >> info.available;
        argument >> info.enabled;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator<<(QDBusArgument& argument, const ClientDeviceInfo& info) {
        argument.beginStructure();
        argument << info.cardName;
        argument << info.cardDesc;
        argument << info.portName;
        argument << info.portLabel;
        argument << info.priority;
        argument << info.direction;
        argument << info.available;
        argument << info.enabled;
        argument.endStructure();
        return argument;
    }
} ClientDeviceInfo;

Q_DECLARE_METATYPE(ClientDeviceInfo)

// 注册 ClientDefaultDeviceInfo 类型
typedef struct ClientDefaultDeviceInfo
{
    quint32 idx;             // 默认设备的索引
    QString name;            // 默认设备的名称(sink/source 名称)
    QString cardName;        // 默认设备所在的声卡名称
//    QString cardDesc;        // 默认设备所在的声卡描述
    QString activePortName;  // 默认设备的活跃端口名称，可能为空
    QString activePortDesc;  // 默认设备的活跃端口描述
    friend const QDBusArgument& operator>>(const QDBusArgument& argument, ClientDefaultDeviceInfo& info) {
        argument.beginStructure();
        argument >> info.idx;
        argument >> info.name;
        argument >> info.cardName;
//        argument >> info.cardDesc;
        argument >> info.activePortName;
        argument >> info.activePortDesc;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator<<(QDBusArgument& argument, const ClientDefaultDeviceInfo& info) {
        argument.beginStructure();
        argument << info.idx;
        argument << info.name;
        argument << info.cardName;
//        argument << info.cardDesc;
        argument << info.activePortName;
        argument << info.activePortDesc;
        argument.endStructure();
        return argument;
    }
} ClientDefaultDeviceInfo;

Q_DECLARE_METATYPE(ClientDefaultDeviceInfo)

// 注册 StreamInfo 类型
typedef struct StreamInfo
{
    QString name;     // 流媒体名称
    quint32 index;    // 流媒体索引
    uint32_t volume;  // 流媒体音量
    QString role;     // 流媒体角色
    QString iconName; // 流媒体图标名称

    friend const QDBusArgument& operator>>(const QDBusArgument& argument, StreamInfo& info) {
        argument.beginStructure();
        argument >> info.name;
        argument >> info.index;
        argument >> info.volume;
        argument >> info.role;
        argument >> info.iconName;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator<<(QDBusArgument& argument, const StreamInfo& info) {
        argument.beginStructure();
        argument << info.name;
        argument << info.index;
        argument << info.volume;
        argument << info.role;
        argument << info.iconName;
        argument.endStructure();
        return argument;
    }
} StreamInfo;

Q_DECLARE_METATYPE(StreamInfo)

typedef struct ClientMediaInfo
{
    uint32_t direction;    // 流媒体的direction, input or output
    uint32_t index;        // 流媒体索引
    uint32_t volume;       // 流媒体音量值
    bool mute;             // 流媒体静音状态
    QString name;          // 流媒体名称

    friend const QDBusArgument& operator>>(const QDBusArgument& argument, ClientMediaInfo& info) {
        argument.beginStructure();
        argument >> info.direction;
        argument >> info.index;
        argument >> info.volume;
        argument >> info.mute;
        argument >> info.name;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator<<(QDBusArgument& argument, const ClientMediaInfo& info) {
        argument.beginStructure();
        argument << info.direction;
        argument << info.index;
        argument << info.volume;
        argument << info.mute;
        argument << info.name;
        argument.endStructure();
        return argument;
    }
} ClientMediaInfo;

Q_DECLARE_METATYPE(ClientMediaInfo)

typedef struct ClientStreamMediaInfo
{
    QString mediaRole;                        // 流媒体角色
    QString iconName;                         // 流媒体应用图标名称
    QString binary;                           // 流媒体对应的二进制名称
    std::list<ClientMediaInfo> mediaInfoList; // 流媒体信息

    friend const QDBusArgument& operator>>(const QDBusArgument& argument, ClientStreamMediaInfo& info) {
        argument.beginStructure();
        argument >> info.mediaRole;
        argument >> info.iconName;
        argument >> info.binary;

        argument.beginArray();
        while (!argument.atEnd()) {
            ClientMediaInfo element;
            argument >> element;
            info.mediaInfoList.push_back(element);
        }
        argument.endArray();
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator<<(QDBusArgument& argument, const ClientStreamMediaInfo& info) {
        argument.beginStructure();
        argument << info.mediaRole;
        argument << info.iconName;
        argument << info.binary;
        argument.beginArray();
        for (const auto& media : info.mediaInfoList) {
            argument << media;
        }
        argument.endArray();
        argument.endStructure();
        return argument;
    }
} ClientStreamMediaInfo;

Q_DECLARE_METATYPE(ClientStreamMediaInfo)

// 注册 ClientSoundThemeInfo 类型
typedef struct ClientSoundThemeInfo
{
    QString name;           // 音效主题的名称
    QString description;    // 音效主题的描述

    friend const QDBusArgument& operator>>(const QDBusArgument& argument, ClientSoundThemeInfo& info) {
        argument.beginStructure();
        argument >> info.name;
        argument >> info.description;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator<<(QDBusArgument& argument, const ClientSoundThemeInfo& info) {
        argument.beginStructure();
        argument << info.name;
        argument << info.description;
        argument.endStructure();
        return argument;
    }
} ClientSoundThemeInfo;

Q_DECLARE_METATYPE(ClientSoundThemeInfo)

// 注册 ClientSoundEffectFileInfo 类型
typedef struct ClientSoundEffectFileInfo
{
    QString name;           // 音效主题的名称
    QString description;    // 音效主题的描述

    friend const QDBusArgument& operator>>(const QDBusArgument& argument, ClientSoundEffectFileInfo& info) {
        argument.beginStructure();
        argument >> info.name;
        argument >> info.description;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator<<(QDBusArgument& argument, const ClientSoundEffectFileInfo& info) {
        argument.beginStructure();
        argument << info.name;
        argument << info.description;
        argument.endStructure();
        return argument;
    }
} ClientSoundEffectFileInfo;

Q_DECLARE_METATYPE(ClientSoundEffectFileInfo)

class DBusClient : public QObject
{
    Q_OBJECT
public:
    static DBusClient& getInstance();

public:
    void initDbusConnect();

    template <typename...Args>
    QDBusMessage dbusMethodCall(const QString&, const QString&, const QString&, const QString&, Args &&...);

Q_SIGNALS:
    void volumeChangedSignal(int, int, const QDBusVariant&);

    void muteChangedSignal(int, int, bool);

    void deviceChangedSignal(int, const QString&, const QString&);

    void deviceAdjustSignal(int);

    void settingsChangedSignal(const QString&, const QDBusVariant&);

    void addStreamSignal(int, int, int, bool, const QString&, const QString&);

    void removeStreamSignal(int, int);

private Q_SLOTS:
    void volumeChangedSlots(int, int, const QDBusVariant&);

    void muteChangedSlots(int, int, bool);

    void deviceChangedSlots(int, const QString&, const QString&);

    void deviceAdjustSlots(int);

    void settingsChangedSlots(const QString&, const QDBusVariant&);

    void addStreamSlots(int, int, int, bool, const QString&, const QString&);

    void removeStreamSlots(int, int);

private:
    DBusClient() = default;
    DBusClient(const DBusClient&) = delete;
    DBusClient(DBusClient&&) = delete;
    DBusClient operator=(const DBusClient&) = delete;
    DBusClient operator=(DBusClient&&) = delete;
    ~DBusClient() = default;
};

template <typename... Args>
QDBusMessage DBusClient::dbusMethodCall(const QString& destination, const QString& path, const QString& interface, const QString& method, Args&&... args)
{
    QDBusInterface mediaInterface(destination, path, interface);
    return mediaInterface.call(method, std::forward<Args>(args)...);
}

#endif // DBUSCLIENT_H
