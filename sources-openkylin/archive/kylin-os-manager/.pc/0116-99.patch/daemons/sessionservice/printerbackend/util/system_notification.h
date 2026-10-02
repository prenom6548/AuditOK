#ifndef SYSTEM_NOTIFICATION_H
#define SYSTEM_NOTIFICATION_H

#include <QObject>

class QDBusInterface;

class SystemNotification : public QObject
{
    Q_OBJECT
public:
    explicit SystemNotification(QObject *parent = nullptr);

    void sendMessage(const QString &text);

    // timeout :
    // -1 : 表示消息通知常驻,不会收纳进通知中心
    //  0 : 表示通知将直接进入侧边栏通知中心，不会弹窗
    // >0 : 表示通知弹窗持续时间
    void sendMessage(const QString &text, const QString &title, int timeout);

    void sendMessage(const QString &text, const QString &title, int timeout, const QStringList &actions);

private:
    QDBusInterface *m_interface;
};

#endif // SYSTEM_NOTIFICATION_H