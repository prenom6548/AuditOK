#include "boot_partition_cleaner.h"
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDebug>

namespace junk_clean {

BootPartitionCleaner::BootPartitionCleaner(QObject *parent)
    : Cleaner(parent)
{
    QDBusConnection::systemBus().connect(JUNK_CLEAN_DBUS_NAME,
                                         JUNK_CLEAN_DBUS_PATH,
                                         JUNK_CLEAN_DBUS_INTERFACE,
                                         "ScanForJunk",
                                         this,
                                         SLOT(on_ScanForJunk(QString,quint64,QString,quint64)));
    QDBusConnection::systemBus().connect(JUNK_CLEAN_DBUS_NAME,
                                         JUNK_CLEAN_DBUS_PATH,
                                         JUNK_CLEAN_DBUS_INTERFACE,
                                         "ScanFinish",
                                         this,
                                         SLOT(on_ScanFinish(QString,quint64)));
    QDBusConnection::systemBus().connect(JUNK_CLEAN_DBUS_NAME,
                                         JUNK_CLEAN_DBUS_PATH,
                                         JUNK_CLEAN_DBUS_INTERFACE,
                                         "CleanForJunk",
                                         this,
                                         SLOT(on_CleanForJunk(QString,quint64)));
    QDBusConnection::systemBus().connect(JUNK_CLEAN_DBUS_NAME,
                                         JUNK_CLEAN_DBUS_PATH,
                                         JUNK_CLEAN_DBUS_INTERFACE,
                                         "CleanFinish",
                                         this,
                                         SLOT(on_CleanFinish(QString)));
}

QString BootPartitionCleaner::Mark() const {
    return "boot_partition_cleaner";
}

enum Type BootPartitionCleaner::Type() const {
    return Type::kSystemJunk;
}

enum Level BootPartitionCleaner::Level() const {
    return Level::kNoEffect;
}

QString BootPartitionCleaner::Name() const {
    return tr("boot partition");
}

QString BootPartitionCleaner::Description() const {
    return tr("Boot partition space cleaning");
}

void BootPartitionCleaner::Scan() {
    const auto message = QDBusMessage::createMethodCall(JUNK_CLEAN_DBUS_NAME,
                                                        JUNK_CLEAN_DBUS_PATH,
                                                        JUNK_CLEAN_DBUS_INTERFACE,
                                                        "ScanBootPartition");
    const auto reply = QDBusConnection::systemBus().call(message);
    if (reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        qCritical() << "Junk clean scan boot partition d-bus interface call fail: " << reply.errorMessage();
        Q_EMIT sig_ScanFinish(Mark(), 0);
    }
}

void BootPartitionCleaner::Clean(QList<quint64> junk_marks) {
    auto message = QDBusMessage::createMethodCall(JUNK_CLEAN_DBUS_NAME,
                                                  JUNK_CLEAN_DBUS_PATH,
                                                  JUNK_CLEAN_DBUS_INTERFACE,
                                                  "CleanBootPartition");
    QList<QVariant> args;
    args.push_back(QVariant::fromValue(junk_marks));
    message.setArguments(args);
    auto reply = QDBusConnection::systemBus().call(message);
    if (reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        qCritical() << "Junk clean clean boot partition d-bus interface call fail: " << reply.errorMessage();
        Q_EMIT sig_CleanFinish(Mark());
    }
}

void BootPartitionCleaner::on_ScanForJunk(QString cleaner_mark, quint64 junk_mark, QString path, quint64 size) {
    if (cleaner_mark != Mark()) {
        return;
    }
    JunkItem junk_item;
    junk_item.mark = junk_mark;
    junk_item.path = path;
    junk_item.size = size;
    Q_EMIT sig_ScanForJunk(Mark(), junk_item);
}

void BootPartitionCleaner::on_ScanFinish(QString cleaner_mark, quint64 total_size) {
    if (cleaner_mark != Mark()) {
        return;
    }
    Q_EMIT sig_ScanFinish(Mark(), total_size);
}

void BootPartitionCleaner::on_CleanForJunk(QString cleaner_mark, quint64 junk_mark) {
    if (cleaner_mark != Mark()) {
        return;
    }
    Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
}

void BootPartitionCleaner::on_CleanFinish(QString cleaner_mark) {
    if (cleaner_mark != Mark()) {
        return;
    }
    Q_EMIT sig_CleanFinish(Mark());
}

}
