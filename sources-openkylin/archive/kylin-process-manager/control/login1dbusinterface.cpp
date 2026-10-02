// #include "login1dbusinterface.h"
// #include "loghelper.h"
// namespace {
//     const char *login1_dbus_service = "org.freedesktop.login1";
//     const char *login1_dbus_path = "/org/freedesktop/login1";
//     const char *login1_manager_interface = "org.freedesktop.login1.Manager";
//     //const char *dbus_properties_interface = "org.freedesktop.DBus.Properties";
// }

// // 2. 实现D-Bus序列化/反序列化运算符（必须与a(susso)签名严格对应）
// QDBusArgument& operator<<(QDBusArgument& arg, const Login1Session& session) {
//     arg.beginStructure();
//     // 严格按照 s(字符串) + u(无符号整数) + s(字符串) + s(字符串) + o(对象路径) 的顺序
//     arg << session.id;
//     arg << session.uid;
//     arg << session.seatId;
//     arg << session.sessionName;
//     arg << session.objectPath;
//     arg.endStructure();
//     return arg;
// }

// const QDBusArgument& operator>>(const QDBusArgument& arg, Login1Session& session) {
//     arg.beginStructure();
//     // 与序列化顺序完全一致，否则解析会错位
//     arg >> session.id;
//     arg >> session.uid;
//     arg >> session.seatId;
//     arg >> session.sessionName;
//     arg >> session.objectPath;
//     arg.endStructure();
//     return arg;
// }

// Login1DbusInterface::Login1DbusInterface()
//     : m_login1ManagerInterface(login1_dbus_service, login1_dbus_path, login1_manager_interface, QDBusConnection::systemBus())
// {
//     if (!m_login1ManagerInterface.isValid()) {
//             PRINT_DBG("Login1DbusInterface::Login1DbusInterface(): create login1 manager interface failed : %s\n", m_login1ManagerInterface.lastError().message().toUtf8().constData());
//     }
//     registerMetaType();
// }

// Login1DbusInterface::~Login1DbusInterface()
// {
// }

// // // 获取所有会话信息
// // QList<Login1Session> Login1DbusInterface::getSessions() {
// //     QList<Login1Session> sessions;

// //     if (!m_login1ManagerInterface.isValid()) {
// //         return sessions;
// //     }

// //     // 调用D-Bus方法
// //     QDBusReply<QList<QVariantList>> reply = m_login1ManagerInterface.call("ListSessions");
// //     if (!reply.isValid()) {
// //         qWarning() << "调用ListSessions失败:" << reply.error().message();
// //         return sessions;
// //     }

// //     // 解析返回结果
// //     const auto& sessionLists = reply.value();
// //     for (const auto& sessionData : sessionLists) {
// //         if (sessionData.size() >= 4) {
// //             Login1Session session;
// //             session.id = sessionData[0].toString();
// //             session.uid = sessionData[1].toUInt();
// //             session.seatId = sessionData[2].toString();
// //             session.objectPath = sessionData[3].toString();
// //             sessions.append(session);
// //         } else {
// //             qWarning() << "会话数据格式错误，字段数量不足";
// //         }
// //     }

// //     return sessions;
// // }

// QList<Login1Session> Login1DbusInterface::getSessions() {
//         QList<Login1Session> sessions;

//         // 关键：使用注册的QList<Login1Session>作为返回类型，直接匹配a(susso)签名
//         QDBusReply<QList<Login1Session>> reply = m_login1ManagerInterface.call("ListSessions");
//         if (!reply.isValid()) {
//             qWarning() << "调用ListSessions失败:" << reply.error().message();
//             return sessions;
//         }

//         // 直接返回解析后的结果（Qt已通过注册的元类型自动解析）
//         return reply.value();
//     }

// // 打印所有会话信息
// void Login1DbusInterface::printSessions() {
//     auto sessions = getSessions();
//     if (sessions.isEmpty()) {
//         qDebug() << "未获取到会话信息";
//         return;
//     }

//     qDebug() << "当前会话列表: ";
//     for (const auto& session : sessions) {
//         qDebug() << session.toString();
//     }
// }

// void Login1DbusInterface::registerMetaType()
// {
//     qRegisterMetaType<Login1Session>("Login1Session");
//     qRegisterMetaType<Login1SessionList>("Login1SessionList");
//     qDBusRegisterMetaType<Login1Session>();
//     qDBusRegisterMetaType<Login1SessionList>();
// }
#include "login1dbusinterface.h"
#include "loghelper.h"
#include <QDBusMetaType>

namespace
{
const char *login1_dbus_service = "org.freedesktop.login1";
const char *login1_dbus_path = "/org/freedesktop/login1";
const char *login1_manager_interface = "org.freedesktop.login1.Manager";

const char *properties_interface = "org.freedesktop.DBus.Properties";
const char *login1_session_interface = "org.freedesktop.login1.Session";
}

QDBusUnixFileDescriptor Login1DbusInterface::m_inhibitFileDescriptor = QDBusUnixFileDescriptor();

// 全局注册元类型（在任何DBus调用之前）
static void registerLogin1MetaTypes()
{
    static bool registered = false;
    if (!registered) {
        qRegisterMetaType<Login1Session>("Login1Session");
        qRegisterMetaType<Login1SessionList>("Login1SessionList");
        qDBusRegisterMetaType<Login1Session>();
        qDBusRegisterMetaType<Login1SessionList>();
        registered = true;
    }
}

// DBus序列化/反序列化运算符
QDBusArgument &operator<<(QDBusArgument &arg, const Login1Session &session)
{
    arg.beginStructure();
    arg << session.id; // s
    arg << session.uid; // u
    arg << session.seatId; // s
    arg << session.sessionName; // s - 这个字段可能需要调整
    arg << session.objectPath; // o
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, Login1Session &session)
{
    arg.beginStructure();
    arg >> session.id;
    arg >> session.uid;
    arg >> session.seatId;
    arg >> session.sessionName;
    arg >> session.objectPath;
    arg.endStructure();
    return arg;
}

Login1DbusInterface::Login1DbusInterface()
{
    // 先注册元类型
    registerLogin1MetaTypes();

    // 再创建DBus接口
    // m_login1ManagerInterface = QDBusInterface(login1_dbus_service,
    //                                         login1_dbus_path,
    //                                         login1_manager_interface,
    //                                         QDBusConnection::systemBus());
    m_login1ManagerInterface = new QDBusInterface(login1_dbus_service,
                                                  login1_dbus_path,
                                                  login1_manager_interface,
                                                  QDBusConnection::systemBus());

    if (!m_login1ManagerInterface->isValid()) {
        PRINT_DBG("Login1DbusInterface: create login1 manager interface failed: %s\n",
                  m_login1ManagerInterface->lastError().message().toUtf8().constData());
    }
}

Login1DbusInterface::~Login1DbusInterface()
{
    if (!m_login1ManagerInterface) {
        delete m_login1ManagerInterface;
    }
}

QList<Login1Session> Login1DbusInterface::getSessions()
{
    QList<Login1Session> sessions;

    if (!m_login1ManagerInterface->isValid()) {
        qWarning() << "DBus接口无效";
        return sessions;
    }
    // 方法1: 使用注册的元类型（推荐）
    QDBusReply<Login1SessionList> reply = m_login1ManagerInterface->call("ListSessions");
    if (!reply.isValid()) {
        qWarning() << "调用ListSessions失败:" << reply.error().message();
        // 如果元类型方式失败，尝试原始方式
        sessions = getSessionsFallback();
    } else {
        sessions = reply.value();
    }
    for (auto &session : sessions) {
        session.isRemote = isRemoteSession(session.objectPath.path());
    }
    return sessions;
}

// 备选方案：使用原始QVariant解析
QList<Login1Session> Login1DbusInterface::getSessionsFallback()
{
    QList<Login1Session> sessions;

    QDBusMessage message = QDBusMessage::createMethodCall(login1_dbus_service,
                                                          login1_dbus_path,
                                                          login1_manager_interface,
                                                          "ListSessions");

    QDBusMessage reply = QDBusConnection::systemBus().call(message);

    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "DBus调用失败:" << reply.errorMessage();
        return sessions;
    }
    if (reply.arguments().isEmpty()) {
        qWarning() << "没有返回参数";
        return sessions;
    }

    // 解析原始DBus数据
    QVariant firstArg = reply.arguments().first();
    qDebug() << "原始返回类型:" << firstArg.typeName();

    if (firstArg.canConvert<QDBusArgument>()) {
        QDBusArgument arg = firstArg.value<QDBusArgument>();
        qDebug() << "DBus签名:" << arg.currentSignature();

        if (arg.currentType() == QDBusArgument::ArrayType) {
            arg.beginArray();
            while (!arg.atEnd()) {
                // 尝试解析每个结构体
                if (arg.currentType() == QDBusArgument::StructureType) {
                    Login1Session session;
                    arg >> session;
                    sessions.append(session);
                } else {
                    qWarning() << "非结构体类型，跳过";
                    break;
                }
            }
            arg.endArray();
        }
    }

    return sessions;
}

void Login1DbusInterface::printSessions()
{
    auto sessions = getSessions();
    if (sessions.isEmpty()) {
        qDebug() << "未获取到会话信息";
        debugRawDBusReply(); // 调试原始返回
        return;
    }

    qDebug() << "当前会话列表 (共" << sessions.count() << "个会话):";
    for (const auto &session : sessions) {
        qDebug() << session.toString();
        if (isRemoteSession(session.objectPath.path())) {
            qDebug() << session.sessionName << " is remote session";
        }
    }
}

// 调试方法：打印原始DBus返回
void Login1DbusInterface::debugRawDBusReply()
{
    qDebug() << "=== 调试原始DBus返回 ===";

    QDBusMessage message = QDBusMessage::createMethodCall(login1_dbus_service,
                                                          login1_dbus_path,
                                                          login1_manager_interface,
                                                          "ListSessions");

    QDBusMessage reply = QDBusConnection::systemBus().call(message);

    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "错误:" << reply.errorMessage();
        return;
    }

    qDebug() << "参数数量:" << reply.arguments().count();

    for (int i = 0; i < reply.arguments().count(); ++i) {
        QVariant arg = reply.arguments().at(i);
        qDebug() << "参数" << i << "类型:" << arg.typeName();

        if (arg.canConvert<QDBusArgument>()) {
            QDBusArgument dbusArg = arg.value<QDBusArgument>();
            qDebug() << "DBus签名:" << dbusArg.currentSignature();
            qDebug() << "DBus类型:" << dbusArg.currentType();
        }
    }
}

bool Login1DbusInterface::isRemoteSession(QString sessionObjectPath)
{
    QDBusInterface sessionInterface(login1_dbus_service, sessionObjectPath, properties_interface, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = sessionInterface.call("Get", login1_session_interface, "Remote");
    if (!reply.isValid()) {
        qWarning() << "resume Compositor  failed, " << reply.error();
        return false;
    }
    return reply.value().toBool();
}

void Login1DbusInterface::inhibit()
{
    if (m_inhibitFileDescriptor.isValid()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                                          "/org/freedesktop/login1",
                                                          "org.freedesktop.login1.Manager",
                                                          QStringLiteral("Inhibit"));
    message.setArguments(QVariantList({QStringLiteral("sleep"),
                                       "Process Manager Idle",
                                       "Make sure that the frozen state does not affect sleep.",
                                       QStringLiteral("delay")}));
    QDBusPendingReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(message);
    if (!reply.isValid()) {
        return;
    }
    reply.value().swap(m_inhibitFileDescriptor);
    PRINT_NOTICE("inhibit\n");
}

void Login1DbusInterface::uninhibit()
{
    if (!m_inhibitFileDescriptor.isValid()) {
        return;
    }

    m_inhibitFileDescriptor = QDBusUnixFileDescriptor();
    PRINT_NOTICE("uninhibit\n");
}
