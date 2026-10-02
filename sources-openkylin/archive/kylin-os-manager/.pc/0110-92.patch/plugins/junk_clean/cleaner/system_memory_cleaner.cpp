#include "system_memory_cleaner.h"
#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>

namespace junk_clean {

SystemMemoryCleaner::SystemMemoryCleaner(QObject *parent)
    : Cleaner(parent),
      dbus_system_memory_cleaner_mark_("system_memory_cleaner")
{
    QDBusConnection::systemBus().connect(JUNK_CLEAN_DBUS_NAME,
                                         JUNK_CLEAN_DBUS_PATH,
                                         JUNK_CLEAN_DBUS_INTERFACE,
                                         "CleanFinish",
                                         this,
                                         SLOT(on_CleanFinish(QString)));
}

QString SystemMemoryCleaner::Mark() const {
    return "system_memory_cleaner";
}

enum Type SystemMemoryCleaner::Type() const {
    return Type::kSystemJunk;
}

enum Level SystemMemoryCleaner::Level() const {
    return Level::kNoEffect;
}

QString SystemMemoryCleaner::Name() const {
    return tr("System cache");
}

QString SystemMemoryCleaner::Description() const {
    return tr("Release system memory");
}

void SystemMemoryCleaner::Scan() {
    Q_EMIT sig_ScanFinish(Mark(), 1);
}

void SystemMemoryCleaner::Clean([[maybe_unused]]QList<quint64> junk_marks) {
    auto message = QDBusMessage::createMethodCall(JUNK_CLEAN_DBUS_NAME,
                                                  JUNK_CLEAN_DBUS_PATH,
                                                  JUNK_CLEAN_DBUS_INTERFACE,
                                                  "CleanSystemMemory");
    auto reply = QDBusConnection::systemBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "System memory cleaner clean call d-bus interface fail.";
        Q_EMIT sig_CleanFinish(Mark());
    }
}

void SystemMemoryCleaner::on_CleanFinish(QString clean_mark) {
    if (clean_mark != dbus_system_memory_cleaner_mark_) {
        return;
    }
    Q_EMIT sig_CleanFinish(Mark());
}

}
