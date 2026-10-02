#ifndef DBUS_H
#define DBUS_H

#include <QObject>
#include "dbus_args.h"
namespace dbus
{

class Tool : public QObject
{
    Q_OBJECT
public:
    Tool(QObject *parent = nullptr) : QObject(parent) {}
public slots:
    QStringList getFileNameFromDir(const QString &arg) const;
};

class Dbus : public QObject
{
    Q_OBJECT
public:
    Dbus(QObject *parent = nullptr) : QObject(parent) {}
public slots:
    void creatDbusServer();
};

class DbusManager : public QObject
{
    Q_OBJECT
signals:
    void creatDbusServer();

public:
    DbusManager();
};

} // namespace dbus

#endif // DBUS_H
