#include "installation_package_cleaner.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QMetaType>
#include <QDBusMetaType>
#include <QDebug>

namespace junk_clean {

InstallationPackageCleaner::InstallationPackageCleaner(QObject *parent)
    : Cleaner(parent)
{
    qRegisterMetaType<QList<quint64>>();
    qDBusRegisterMetaType<QList<quint64>>();

    QDBusConnection::systemBus().connect(JUNK_CLEAN_DBUS_NAME,
                                         JUNK_CLEAN_DBUS_PATH,
                                         JUNK_CLEAN_DBUS_INTERFACE,
                                         "ScanForJunk",
                                         this,
                                         SLOT(on_ScanForJunk(QString, quint64, QString, quint64)));
    QDBusConnection::systemBus().connect(JUNK_CLEAN_DBUS_NAME,
                                         JUNK_CLEAN_DBUS_PATH,
                                         JUNK_CLEAN_DBUS_INTERFACE,
                                         "ScanFinish",
                                         this,
                                         SLOT(on_ScanFinish(QString, quint64)));
    QDBusConnection::systemBus().connect(JUNK_CLEAN_DBUS_NAME,
                                         JUNK_CLEAN_DBUS_PATH,
                                         JUNK_CLEAN_DBUS_INTERFACE,
                                         "CleanForJunk",
                                         this,
                                         SLOT(on_CleanForJunk(QString, quint64)));
    QDBusConnection::systemBus().connect(JUNK_CLEAN_DBUS_NAME,
                                         JUNK_CLEAN_DBUS_PATH,
                                         JUNK_CLEAN_DBUS_INTERFACE,
                                         "CleanFinish",
                                         this,
                                         SLOT(on_CleanFinish(QString)));
}

QString InstallationPackageCleaner::Mark() const {
    return "installation_package_cleaner";
}

enum Type InstallationPackageCleaner::Type() const {
    return Type::kSystemJunk;
}

enum Level InstallationPackageCleaner::Level() const {
    return Level::kNoEffect;
}

QString InstallationPackageCleaner::Name() const {
    return tr("Installation package cache");
}

QString InstallationPackageCleaner::Description() const {
    return tr("Garbage files generated during software installation");
}

void InstallationPackageCleaner::Scan() {
    auto message = QDBusMessage::createMethodCall(JUNK_CLEAN_DBUS_NAME,
                                                  JUNK_CLEAN_DBUS_PATH,
                                                  JUNK_CLEAN_DBUS_INTERFACE,
                                                  "ScanInstallationPackage");
    auto reply = QDBusConnection::systemBus().call(message);
    if (reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        qCritical() << "Junk clean scan installation package d-bus interface call fail.";
        Q_EMIT sig_ScanFinish(Mark(), 0);
    }
}

void InstallationPackageCleaner::Clean(QList<quint64> junk_marks) {
    auto message = QDBusMessage::createMethodCall(JUNK_CLEAN_DBUS_NAME,
                                                  JUNK_CLEAN_DBUS_PATH,
                                                  JUNK_CLEAN_DBUS_INTERFACE,
                                                  "CleanInstallationPackage");
    QList<QVariant> args;
    args.push_back(QVariant::fromValue(junk_marks));
    message.setArguments(args);
    auto reply = QDBusConnection::systemBus().call(message);
    if (reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        qCritical() << "Junk clean clean installation package d-bus interface call fail.";
        Q_EMIT sig_CleanFinish(Mark());
    }
}

void InstallationPackageCleaner::on_ScanForJunk(QString cleaner_mark, quint64 junk_mark, QString path, quint64 size) {
    if (cleaner_mark != "installation_package_cleaner") {
        return;
    }
    JunkItem junk_item;
    junk_item.mark = junk_mark;
    junk_item.path = path;
    junk_item.size = size;
    Q_EMIT sig_ScanForJunk(Mark(), junk_item);
}

void InstallationPackageCleaner::on_ScanFinish(QString cleaner_mark, quint64 total_size) {
    if (cleaner_mark != "installation_package_cleaner") {
        return;
    }
    Q_EMIT sig_ScanFinish(Mark(), total_size);
}

void InstallationPackageCleaner::on_CleanForJunk(QString cleaner_mark, quint64 junk_mark) {
    if (cleaner_mark != "installation_package_cleaner") {
        return;
    }
    Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
}

void InstallationPackageCleaner::on_CleanFinish(QString cleaner_mark) {
    if (cleaner_mark != "installation_package_cleaner") {
        return;
    }
    Q_EMIT sig_CleanFinish(Mark());
}

}
