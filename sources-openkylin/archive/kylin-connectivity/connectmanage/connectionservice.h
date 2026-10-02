#ifndef CONNECTIONSERVICE_H
#define CONNECTIONSERVICE_H

#include <QObject>
#include <QMap>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QFile>

#include "serverstatus.h"
#include "messageserialize.pb.h"
#include "adbprocess.h"

typedef std::function<void()> ConnectSuccessCallBack;  // 定义连接成功函数类型
typedef std::function<void()> ConnectFailCallBack;     // 定义连接失败函数类型

class ConnectionService : public QObject
{
    Q_OBJECT
public:
    ConnectionService(QObject *parent = nullptr);
    ~ConnectionService();

    // wifi连接
    // 启动客户端连接服务器
    bool startClient(const QString host);
    // 设置连接是否成功回调函数
    void setConnectionCallback(ConnectSuccessCallBack connectSuccessCallBack, ConnectFailCallBack connectFailCallBack);
    // 尝试连接address:port主机地址，使用前需设置成功和失败的回调函数
    void connectRemoteHost(const QString address);
    // 设置验证结果
    void setConnectionRespond(bool isAgree);
    // 断开连接
    bool abortService();
    // 发送字节数据
    bool sendByteData(const QString data);
    bool sendByteData(const char *buffer, const qint64 bufSize);
    // 发送文件数据
    void setFileSavePath(const QString path);
    bool sendFileData(const QString path);
    bool sendFileData(const QStringList pathList);
    // 发送流数据
    bool openStream(bool flag = true);
    bool sendStreamData(char *buffer, qint32 bufSize);

    // USB连接
    void connectLocDev();
    // 是否支持反控
    bool isSupportADB();
    // 获取设备序列号
    QString getDeviceSerial();

    // 业务能力
    bool closeAbility();
    void setUserPassword(QString userName, QString password);

public:
    enum ConnectType {
        NOTCONNECT = 0,  // 未连接
        WIFI,            // WiFi连接
        USB              // USB连接
    };
    Q_ENUM(ConnectType)
    enum DeviceType {
        PC = 0,   // PC端
        ANDROID,  // 安卓端
        UNKNOWN   // 未知设备
    };
    Q_ENUM(DeviceType)
    enum DeviceRole {
        INITIATOR = 0,  // 发起者
        RECIPIENT,      // 接收者
    };
    Q_ENUM(DeviceRole)
    struct FtpNamePwd {
        QString username = "";  // 用户名
        QString pwd = "";       // 密码
    };

    struct ConnectionInfo {
        QString uuid = 0;
        QString deviceName = "";
        QString address = "";
        DeviceType deviceType = DeviceType::UNKNOWN;
        ConnectType connectType = ConnectType::NOTCONNECT;
        FtpNamePwd usernamePwd;
        DeviceRole deviceRole = DeviceRole::INITIATOR;
    };

Q_SIGNALS:
    void sigConnectRequest(QString);
    void sigConnectDenied();
    void sigConnectInfo(const ConnectionInfo &);
    void sigDisconnect();
    // test
    void sigByteReceived(QString);
    void sigFileReceived(QString);
    // usb
    void sigNotFountApk();
    void sigDeviceRedundancy();
    void sigNoUsbDevice();
public Q_SLOTS:
    void slotServerNewConnection();
    void onMessageReadyRead();
    void onDataReadyRead();
    void onHeartbeatReadyRead();
    void slotServerState(bool state);
    void onAdbExec(AdbProcess::ADB_EXEC_RESULT processResult);
    void updateSocketState();
    void slotHeartbeatConnection();

private:
    enum DataType {
        FILE_DATA = 0,  // 文件内容
        STREAM_DATA,    // 流数据
        BYTE_DATA,      // 字节数据
        FREE            // 空闲
    };

    enum AndroidConnType {
        ADB = 0,          // 有线adb
        CONNECTWIFI = 1,  // wifi
        WIFIADB = 2,      // 无线adb
        NOTANDROID = 3    // 不是安卓设备
    };

private:
    void deleteSocket();
    void disconnectSocket();
    bool sendMessage(const ReplyMessage &message);
    bool sendBusyMessage();
    void openWriteFile();
    void endFile();
    bool sendStartAudioMessage();
    void clearConnectionInfo();
    void updateDevice();
    void checkApk(QString serials);
    void setMapPort();
    void startApp(QString serials);
    bool isSameDevice(QString serials);
    void handleWifiConnection();
    void sendDeviceInfo();
    void receiveDeviceInfo(const ReplyMessage &reply);
    void connectDataSocket();
    void startHeartbeat();
    void connectHeartbeat();
    QString getSocketAddress();
    void sendHeartbeat(bool isServer);
    void handleConnected();

private:
    // wifi
    QTcpServer *m_tcpServer = nullptr;                          // Tcp服务
    QTcpSocket *m_messageTcpSocket = nullptr;                   // 消息通道
    QTcpSocket *m_dataTcpSocket = nullptr;                      // 数据通道
    QTcpServer *m_heartTcpServer = nullptr;                     // 心跳机制服务
    QTcpSocket *m_heartbeatSocket = nullptr;                    // 心跳通道
    ServerStatus *m_serverStatus = nullptr;                     // 判断服务器状态
    QThread *m_statusThread = nullptr;                          // 判断服务器状态线程
    ConnectSuccessCallBack m_connectSuccessCallBack = nullptr;  // 连接成功回调函数
    ConnectFailCallBack m_connectFailCallBack = nullptr;        // 连接失败回调函数
    QFile *m_writeFile = nullptr;
    // usb
    AdbProcess *m_adb = nullptr;                                      // adb接口
    QTimer *m_timer = nullptr;                                        // 获取socket状态定时器

    ConnectionInfo m_connectInfo;                                     // 连接信息
    QString m_serverIp = "";                                          // 服务器主机地址
    int m_port = 0;                                                   // 服务器端口
    QStringList m_filePathList;                                       // 所有文件路径
    ProtobufFilesInfo m_filesInfo;                                    // 所有文件信息
    bool m_isMsgSocketConnect = false;                                // 消息通道是否已连接
    DataType m_dataType = DataType::FREE;                             // 当前数据类型
    qint64 m_dataSize = 0;                                            // 接收的数据大小
    qint64 m_writeSize = 0;                                           // 当前写入文件大小
    bool m_isHead = true;                                             // 是否为数据头部
    int m_currentFileIndex = 0;                                       // 当前文件索引
    QByteArray m_data;                                                // 需要发送的数据
    QString m_fileSavePath = "";                                      // 文件保存路径
    QStringList m_deviceSerialList;                                   // 设备序列号列表
    bool m_isSupportADB = false;                                      // 安卓设备是否支持adb反控
    QString m_userName = "";                                          // ftp用户名
    QString m_password = "";                                          // ftp密码
    AndroidConnType m_androidConnType = AndroidConnType::NOTANDROID;  // 安卓连接类型
    bool m_isClient = false;                                          // 是否为客户端
    bool m_isSurvival = false;                                        // 是否收到心跳
    bool m_isAgainSendHeartbeat = true;                               // 是否重新发送
    bool m_isRemoteDisconnect = false;                                // 是否为远程主机断开
};

#endif  // CONNECTIONSERVICE_H
