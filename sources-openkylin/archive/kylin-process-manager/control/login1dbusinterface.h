// #ifndef LOGIN1_DBUS_INTERFACE_H
// #define LOGIN1_DBUS_INTERFACE_H

// #include <QString>
// #include <QVariant>
// #include <QDBusInterface>
// #include <QDBusReply>
// #include <QDebug>
// #include <QDBusInterface>
// #include <QList>
// //#include <QDbusMetaType>
// #include <QDBusMetaType>
// #include <QMetaType>
// #include <QDBusArgument>
// struct Login1Session {
//     QString id;          // s: 会话ID
//     quint32 uid;         // u: 用户ID
//     QString seatId;      // s: 座位ID
//     QString sessionName; // s: 会话名称（之前忽略的第4个字段）
//     QString objectPath;  // o: 对象路径

//     Login1Session() : id(""), uid(0), seatId(""), sessionName(""), objectPath("") {}
//     Login1Session(const QString i, quint32 u, QString sId, QString sName, QString oPath)
//         : id(i), uid(u), seatId(sId), sessionName(sName), objectPath(oPath)
//     {

//     }
//     QString toString() const {
//         return QString("ID: %1, UID: %2, Seat: %3, Name: %4, Path: %5")
//                 .arg(id).arg(uid).arg(seatId).arg(sessionName).arg(objectPath);
//     }
// };

// Q_DECLARE_METATYPE(Login1Session)
// typedef QList<Login1Session> Login1SessionList;
// Q_DECLARE_METATYPE(Login1SessionList);

// class Login1DbusInterface {
// public:
//     Login1DbusInterface();
//     ~Login1DbusInterface();

//     QList<Login1Session> getSessions();

//     void printSessions();

// private:
//     QDBusInterface m_login1ManagerInterface;
//     void registerMetaType();
// };

// #endif

#ifndef LOGIN1DBUSINTERFACE_H
#define LOGIN1DBUSINTERFACE_H

#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QList>
#include <QObject>
#include <memory>

// 必须在类定义前声明元类型
struct Login1Session;
typedef QList<Login1Session> Login1SessionList;

Q_DECLARE_METATYPE(Login1Session)
Q_DECLARE_METATYPE(Login1SessionList)

// // 定义Login1Session结构体
// struct Login1Session {
//     QString id;           // 会话ID (s)
//     uint uid;             // 用户ID (u)
//     QString seatId;       // 座位ID (s)
//     QString sessionName;  // 会话名称 (s) - 这个字段可能有问题
//     QString objectPath;   // 对象路径 (o)

//     QString toString() const {
//         return QString("Session{id:'%1', uid:%2, seat:'%3', name:'%4', path:'%5'}")
//                 .arg(id).arg(uid).arg(seatId).arg(sessionName).arg(objectPath);
//     }
// };

// 修正的结构体定义 - 匹配 a(susso)
struct Login1Session
{
    QString id; // s - 会话ID
    uint uid; // u - 用户ID
    QString sessionName; // s - 用户名（根据DBus文档，这是第三个字段）
    QString seatId; // s - 座位ID
    QDBusObjectPath objectPath; // o - 对象路径（关键修改！）
    bool isRemote; // b - 是否为远程会话

    QString toString() const
    {
        return QString("Session{id:'%1', uid:%2, user:'%3', seat:'%4', path:'%5'}")
            .arg(id)
            .arg(uid)
            .arg(sessionName)
            .arg(seatId)
            .arg(objectPath.path());
    }
};

// 声明D-Bus序列化操作符
QDBusArgument &operator<<(QDBusArgument &arg, const Login1Session &session);
const QDBusArgument &operator>>(const QDBusArgument &arg, Login1Session &session);

class Login1DbusInterface : public QObject
{
    Q_OBJECT

public:
    Login1DbusInterface();
    ~Login1DbusInterface();

    QList<Login1Session> getSessions();
    QList<Login1Session> getSessionsFallback();
    void printSessions();

    // 添加调试方法
    void debugRawDBusReply();
    bool isRemoteSession(QString sessionObjectPath);
    void inhibit();
    void uninhibit();

private:
    void registerMetaType();
    QDBusInterface *m_login1ManagerInterface;
    static QDBusUnixFileDescriptor m_inhibitFileDescriptor;
};

#endif // LOGIN1DBUSINTERFACE_H