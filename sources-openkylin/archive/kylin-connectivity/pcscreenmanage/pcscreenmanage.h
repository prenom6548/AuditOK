#ifndef PCSCREENMANAGE_H
#define PCSCREENMANAGE_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QThread>
// #include <QDBusMessage>
// #include <QDBusInterface>
#include "screenmanagemsg.pb.h"
#include "pcscreen.h"
#include "curtain.h"
#include "control.h"
#include "suspendtabbar.h"
#include "publicattributes.hpp"

class PcScreenManage : public QObject
{
    Q_OBJECT
public:
    explicit PcScreenManage(QObject *parent = nullptr);
    ~PcScreenManage();

    enum ScreenMsg {
        SharingRequest = 0,   // 投屏请求
        RemoteRejection = 1,  // 远程主机拒绝
        StartedScreen = 2,    // 投屏服务已启动
        ScreenExit = 3,       // 投屏服务退出
        ScreenError = 4,      // 投屏错误
        Disconnect = 5,       // 投屏断开
        Successfully = 6,     // 投屏请求成功
        StartedControl = 7,   // 启动反控
        ExitControl = 8,      // 退出反控
        HideView = 9,         // 隐藏投屏
        ShowView = 10,        // 显示投屏
        Exit = 11,            // 主动退出
        PassiveControl = 12,  // 通过快捷键进入反控
    };
    Q_ENUM(ScreenMsg)
    enum DeviceType {
        PC = 0,       // PC
        Android = 1,  // Android
    };
    Q_ENUM(DeviceType)

    void setTheme(PublicAttributes::Theme theme);

    bool isSupport();

    // 断开通信通道
    void disconnected();
    // 连接服务端
    void connectService(const QString host, PcScreenManage::DeviceType deviceType = PcScreenManage::DeviceType::PC);
    // 设置验证结果
    void setConnectionRespond(bool isAgree);
    // 通知对端进入反控
    void startedControl();
    // 通知对端退出反控
    void exitControl();

    void showView();

private:
    // 获取投屏服务
    void startPcService();
    // 启动幕布
    void startCurtain(QString roomid = "");
    // 启动反控
    void startControl();
    // 退出投屏服务
    void closePcScreenService();
    // 关闭幕布
    void closeCurtain();
    // 关闭反控
    void closeControl();

    int sendMessage(const ScreenManageMsg &message);

Q_SIGNALS:
    void sigRequestReceived(PcScreenManage::ScreenMsg);

private Q_SLOTS:
    void slotServerNewConnection();
    void onMessageReadyRead();
    void handleStateChange(QAbstractSocket::SocketState state);
    void slotSocketDisconnect();
    void slotStateChange(Curtain::States state);
    // void slotEnterControl(bool isControl);

private:
    QTcpServer *m_tcpServer = nullptr;         // Tcp服务
    QTcpSocket *m_messageTcpSocket = nullptr;  // 消息通道
    PCScreen *m_pcScreen = nullptr;            // PC投屏服务
    QThread *m_pcScreenThread = nullptr;       // 投屏服务线程
    Curtain *m_curtain = nullptr;              // 投屏幕布
    Control *m_control = nullptr;              // 反控
    PcScreenManage::DeviceType m_deviceType = PcScreenManage::DeviceType::PC;
    // QDBusInterface *m_dbusInterface = nullptr; // 反控dbus

    QString m_address = "";                                            // 客服端ip
    qint64 m_dataSize = 0;                                             // 接收的数据大小
    QByteArray m_data;                                                 // 需要发送的数据
    bool m_isHead = true;                                              // 是否为消息头
    bool m_isService = true;                                           // 是否为服务端
    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light;  // 主题
    bool m_isAtiveStarted = false;
    bool m_isAtiveEnd = false;
};

#endif  // PCSCREENMANAGE_H