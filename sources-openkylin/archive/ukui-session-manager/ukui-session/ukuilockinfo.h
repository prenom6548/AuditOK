#ifndef UKUILOCKINFO_H
#define UKUILOCKINFO_H

#include <QString>
#include <QDBusArgument>
#include <QVector>

namespace InhibitInfoAndUid {
    struct InhibitorInfo {
        QString name;
        QString icon;
        int uid;
    };

    QDBusArgument &operator<<(QDBusArgument &argument, const InhibitInfoAndUid::InhibitorInfo &mystruct);

    const QDBusArgument &operator>>(const QDBusArgument &argument, InhibitInfoAndUid::InhibitorInfo &mystruct);
}

Q_DECLARE_METATYPE(InhibitInfoAndUid::InhibitorInfo)

namespace InhibitInfo {
    struct InhibitorInfo {
        QString name;
        QString icon;
    };

    QDBusArgument &operator<<(QDBusArgument &argument, const InhibitInfo::InhibitorInfo &mystruct);

    const QDBusArgument &operator>>(const QDBusArgument &argument, InhibitInfo::InhibitorInfo &mystruct);
}

Q_DECLARE_METATYPE(InhibitInfo::InhibitorInfo)

namespace SystemdInhibitor {
    struct Inhibitor {
        QString action;
        QString name;
        QString reason;
        QString mode;
        int uid;
        int pid;
    };

    QDBusArgument &operator<<(QDBusArgument &argument, const SystemdInhibitor::Inhibitor &mystruct);

    const QDBusArgument &operator>>(const QDBusArgument &argument, SystemdInhibitor::Inhibitor &mystruct);
}

Q_DECLARE_METATYPE(SystemdInhibitor::Inhibitor)

namespace WindowInfo {
    /// 窗口信息结构体，用于DBus通信
    struct Info {
        QString wid;    // 窗口标识符
        QString appid; // 应用程序标识符
    };

    QDBusArgument &operator<<(QDBusArgument &argument, const WindowInfo::Info &info);
    const QDBusArgument &operator>>(const QDBusArgument &argument, WindowInfo::Info &info);
}

Q_DECLARE_METATYPE(WindowInfo::Info)

class Ukuilockinfo
{
public:
    Ukuilockinfo();
    ~Ukuilockinfo();

public:
    enum InhibitorType {
        logout,
        suspend,
        shutdown
    };

    enum buttonType {
        switchuserBtn,
        hibernateBtn,
        suspendBtn,
        lockscreenBtn,
        logoutBtn,
        rebootBtn,
        shutdownBtn
    };

    static QVector<InhibitInfoAndUid::InhibitorInfo> listInhibitorInfo(Ukuilockinfo::InhibitorType type);

    static bool getCfgValue(Ukuilockinfo::buttonType button);

private:
    static void getLogoutInhibitor(QVector<InhibitInfoAndUid::InhibitorInfo> &inhibitorVec);

    static void getSystemdInhibitor(QString type, QVector<InhibitInfoAndUid::InhibitorInfo> &inhibitorVec);

    static void findNameAndIcon(QString &inhibitorName, InhibitInfoAndUid::InhibitorInfo &inhibitor);

    static QString getAppLocalName(QString desktopfp);
};

#endif // UKUILOCKINFO_H
