#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <QObject>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDebug>
#include <QString>
#include <QDebug>

class Audio : public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.Devices.Audio")

//    Q_PROPERTY(int sinkVolume READ getSinkVolume WRITE setSinkVolume NOTIFY sinkVolumeChanged)
//    Q_PROPERTY(bool sinkMute READ getSinkMute WRITE setSinkMute NOTIFY sinkMuteChanged)
//    Q_PROPERTY(QString sinkDevice READ getSinkDevice WRITE setSinkDevice NOTIFY sinkDeviceChanged)

//    Q_PROPERTY(int sourceVolume READ getSourceVolume WRITE setSourceVolume NOTIFY sourceVolumeChanged)
//    Q_PROPERTY(bool sourceMute READ getSourceMute WRITE setSourceMute NOTIFY sourceMuteChanged)
//    Q_PROPERTY(QString sourceDevice READ getSourceDevice WRITE setSourceDevice NOTIFY sourceDeviceChanged)

    enum NodeType
    {
        INPUT = 0,
        OUTPUT,
        SINK_INPUT,
        SOURCE_OUTPUT
    };

    // 注册 ClientDefaultDeviceInfo 类型
    typedef struct ClientDefaultDeviceInfo
    {
        quint32 idx;             // 默认设备的索引
        QString name;            // 默认设备的名称(sink/source 名称)
        QString cardName;        // 默认设备所在的声卡名称
        QString activePortName;  // 默认设备的活跃端口名称，可能为空
        QString activePortDesc;  // 默认设备的活跃端口描述
        friend const QDBusArgument& operator>>(const QDBusArgument& argument, ClientDefaultDeviceInfo& info) {
            argument.beginStructure();
            argument >> info.idx;
            argument >> info.name;
            argument >> info.cardName;
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
            argument << info.activePortName;
            argument << info.activePortDesc;
            argument.endStructure();
            return argument;
        }
    } ClientDefaultDeviceInfo;

public:
    explicit Audio(QObject *parent = nullptr);
    ~Audio();

    /**
     * @brief initData 初始化数据
     */
    void initData();

    /**
     * @brief initSignals 初始化dbus信号连接
     */
    void initSignals();

public slots:
    /**
     * @brief getSinkVolume 获取系统输出设备的音量属性值
     * @return  音量值，范围0~100
     */
    int getSinkVolume();

    /**
     * @brief getSinkMute 获取系统输出设备的静音状态属性值
     * @return  true：静音，false：未静音
     */
    bool getSinkMute();

    /**
     * @brief getSinkDevice 获取系统输出设备的设备名属性值
     * @return  系统输出设备的设备名属性值
     */
    QString getSinkDevice();

    /**
     * @brief getSourceVolume 获取系统输入设备的音量属性值
     * @return  音量值，范围0~100
     */
    int getSourceVolume();

    /**
     * @brief getSourceMute 获取系统输入设备的静音状态属性值
     * @return  true：静音，false：未静音
     */
    bool getSourceMute();

    /**
     * @brief getSourceDevice 获取系统输入设备的设备名属性值
     * @return  系统输入设备的设备名属性值
     */
    QString getSourceDevice();



    /**
     * @brief setSinkVolume 设置系统输出设备音量
     * @param volume 音量值，范围0~100
     * @return 是否操作成功 true为成功 false为失败
     */
    bool setSinkVolume(int volume);

    /**
     * @brief setSinkMute 设置系统输出设备静音状态
     * @param mute 静音状态，true为静音，false为取消静音
     * @return 是否操作成功 true为成功 false为失败
     */
    bool setSinkMute(bool mute);

    /**
     * @brief setSinkDevice 设置系统输出设备
     * @param cardName 输入设备的声卡名称
     * @param portName 输入设备的端口名称
     * @return 是否操作成功 true为成功 false为失败
     */
    bool setSinkDevice(QString cardName, QString portName="");

    /**
     * @brief setSourceVolume 设置系统输入设备音量
     * @param volume 音量值，范围0~100
     * @return 是否操作成功 true为成功 false为失败
     */
    bool setSourceVolume(int volume);

    /**
     * @brief setSourceMute 设置系统输入设备静音状态
     * @param mute 静音状态，true为静音，false为取消静音
     * @return 是否操作成功 true为成功 false为失败
     */
    bool setSourceMute(bool mute);

    /**
     * @brief setSourceDevice 设置系统输入设备
     * @param cardName 输入设备的声卡名称
     * @param portName 输入设备的端口名称
     * @return 是否操作成功 true为成功 false为失败
     */
    bool setSourceDevice(QString cardName, QString portName="");

private:
    template <typename...Args>
    QDBusMessage dbusMethodCall(const QString&, const QString&, const QString&, const QString&, Args &&...);

signals:
    /**
     * @brief sinkVolumeChanged 系统输出设备音量改变信号
     * @param volume 音量值，范围0~65536,使用(volume / 65536.0 * 100.0) + 0.5转换为范围0~100
     */
    void sinkVolumeChanged(int volume);

    /**
     * @brief sinkMuteChanged 系统输出设备静音状态改变信号
     * @param mute 静音状态，true为静音，false为取消静音
     */
    void sinkMuteChanged(bool mute);

    /**
     * @brief sinkDeviceChanged 系统输出设备改变信号
     * @param device 输出设备的名称
     */
    void sinkDeviceChanged(QString device);

    /**
     * @brief sourceVolumeChanged 系统输入设备音量改变信号
     * @param volume 音量值，范围0~65536,使用(volume / 65536.0 * 100.0) + 0.5转换为范围0~100
     */
    void sourceVolumeChanged(int volume);

    /**
     * @brief sourceMuteChanged 系统输入设备静音状态改变信号
     * @param mute 静音状态，true为静音，false为取消静音
     */
    void sourceMuteChanged(bool mute);

    /**
     * @brief sourceDeviceChanged 系统输入设备改变信号
     * @param device 输入设备的名称
     */
    void sourceDeviceChanged(QString device);

private:
    int sinkVolume;
    bool sinkMute;
    QString sinkDevice;

    int sourceVolume;
    bool sourceMute;
    QString sourceDevice;

    bool m_isVolumeControlDBusClient = false;
};

#endif   /* __AUDIO_H__ */
