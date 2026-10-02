#ifndef DBUS_H
#define DBUS_H

#include <QObject>
#include <QtDBus>
#include <QDBusContext>

#include "common/common.h"
class DBus : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    explicit DBus(QObject *parent = nullptr);
    ~DBus();
protected:
    void showServiceMsg();
private:
    QString executeLinuxCmd(QString strCmd);
};

#endif // DBUS_H
