#ifndef UKUILOCKINFO_H
#define UKUILOCKINFO_H

#include <QString>
#include <QDBusArgument>
#include <QVector>

namespace InhibitInfo {
    struct InhibitorInfo {
        QString name;
        QString icon;
        int uid;
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

    static QVector<InhibitInfo::InhibitorInfo> listInhibitorInfo(Ukuilockinfo::InhibitorType type);

    static bool getCfgValue(Ukuilockinfo::buttonType button);

private:
    static void getLogoutInhibitor(QVector<InhibitInfo::InhibitorInfo> &inhibitorVec);

    static void getSystemdInhibitor(QString type, QVector<InhibitInfo::InhibitorInfo> &inhibitorVec);

    static void findNameAndIcon(QString &inhibitorName, InhibitInfo::InhibitorInfo &inhibitor);

    static QString getAppLocalName(QString desktopfp);
};

#endif // UKUILOCKINFO_H
