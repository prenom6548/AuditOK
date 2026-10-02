#ifndef DBUS_H
#define DBUS_H

#include <QThread>
#include <QDBusContext>

namespace dbus
{
class Tool : public QObject ,protected QDBusContext
{
    Q_OBJECT
public:
    Tool(QObject *parent = nullptr);
public slots:
    void collectionMessage(int type, const QString &path, int detailed);
    void change3rdpartyConf(const QString &group, const QString &key, const QString &value);
private:
    void collectingFile(const QString &saveDir, const QString &message, int detailed);
    void collectingCmd(const QString &saveDir, const QString &message);
    void collectingOther(const QString &saveDir, const QString &message, const QString &name);
    void collectDiskSmartStatus(const QString &saveDir);
    QString getPathFromJson(QString fn, QString k1, QString v1, QString keyRes, QString k2 = "", QString v2 = "");
    QStringList getFileNameFromDir(const QString &arg) const;
    QString getCmdMessage(const QString &message, bool hasType = false);
    bool isAllowedCaller();
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

private:
    void RegisterStartupService();
};

} // namespace dbus

#endif // DBUS_H
