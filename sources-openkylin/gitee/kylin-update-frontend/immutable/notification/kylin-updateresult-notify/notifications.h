#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QDBusInterface>
#include <QString>
#include <QFile>
class Notifications : public QObject
{
    Q_OBJECT

public:
    Notifications(QObject *parent = nullptr);
    static int notifications(const QString &sigvalue);
    static QString getCurrentLocale();
    ~Notifications();
public slots:
    void onNotificationClosed(uint32_t id,uint32_t reason);
    void onActionInvoked(uint32_t id,const QString &action);
};
#endif // NOTIFICATIONS_H
