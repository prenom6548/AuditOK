#ifndef DBUS_H
#define DBUS_H

#include <QThread>
#include <QDBusContext>
#include "dbus_args.h"
#include "../../plugins/service-support/service-support-backend/src/settings.h"
namespace dbus
{
class Tool : public QObject ,protected QDBusContext
{
    Q_OBJECT
public:
    Tool(QObject *parent = nullptr);
public slots:
    void creatBugForZentao(const QString &title, const QString &type, const QString &name, const QString &number, const QString &key);
    void collectionMessage(int type, const QString &path, int detailed);
private:
    void collectingFile(const QString &saveDir, const QString &message, int detailed);
    void collectingCmd(const QString &saveDir, const QString &message);
    void collectingOther(const QString &saveDir, const QString &message, const QString &name);
    QString getPathFromJson(QString fn, QString k1, QString v1, QString keyRes, QString k2 = "", QString v2 = "");
    QStringList getFileNameFromDir(const QString &arg) const;
    QString getCmdMessage(const QString &message, bool hasType = false);
    bool isAllowedCaller();
signals:
    void creatBugForZentaoResult(QString, QString);
};

class Dbus : public QObject
{
    Q_OBJECT
public:
    Dbus(QObject *parent = nullptr) : QObject(parent) {}
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
