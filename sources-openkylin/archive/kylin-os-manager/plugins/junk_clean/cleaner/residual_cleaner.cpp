#include "residual_cleaner.h"
#include <QProcess>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include "defines.h"

namespace junk_clean {

ResidualCleaner::ResidualCleaner(QObject *parent)
    : Cleaner(parent),
      dbus_uninstall_residual_cleaner_mark_("uninstall_residual_cleaner")
{
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

QString ResidualCleaner::Mark() const {
    return "residual_cleaner";
}

enum Type ResidualCleaner::Type() const {
    return Type::kSystemJunk;
}

enum Level ResidualCleaner::Level() const {
    return Level::kNoEffect;
}

QString ResidualCleaner::Name() const {
    return tr("Uninstall residual");
}

QString ResidualCleaner::Description() const {
    return tr("Residual configuration files and cache after software uninstallation");
}

void ResidualCleaner::Scan() {
    auto messages = QDBusMessage::createMethodCall(JUNK_CLEAN_DBUS_NAME,
                                                   JUNK_CLEAN_DBUS_PATH,
                                                   JUNK_CLEAN_DBUS_INTERFACE,
                                                   "ScanUninstallResidual");
    auto reply = QDBusConnection::systemBus().call(messages);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "Residual cleaner scan call d-bus interface fail.";
        Q_EMIT sig_ScanFinish(Mark(), 0);
    }
}

void ResidualCleaner::Clean(QList<quint64> junk_marks) {
    auto message = QDBusMessage::createMethodCall(JUNK_CLEAN_DBUS_NAME,
                                                  JUNK_CLEAN_DBUS_PATH,
                                                  JUNK_CLEAN_DBUS_INTERFACE,
                                                  "CleanUninstallResidual");
    QList<QVariant> args;
    args.push_back(QVariant::fromValue(junk_marks));
    message.setArguments(args);
    auto reply = QDBusConnection::systemBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "Residual cleaner clean call d-bus interface fail.";
        Q_EMIT sig_CleanFinish(Mark());
    }
}

void ResidualCleaner::on_ScanForJunk(QString clean_mark, quint64 junk_mark, QString path, quint64 size) {
    if (clean_mark != dbus_uninstall_residual_cleaner_mark_) {
        return;
    }

    JunkItem junk_item;
    junk_item.mark = junk_mark;
    junk_item.path = path;
    junk_item.size = size;
    Q_EMIT sig_ScanForJunk(Mark(), junk_item);
}

void ResidualCleaner::on_ScanFinish(QString clean_mark, quint64 total_size) {
    if (clean_mark != dbus_uninstall_residual_cleaner_mark_) {
        return;
    }
    Q_EMIT sig_ScanFinish(Mark(), total_size);
}

void ResidualCleaner::on_CleanForJunk(QString clean_mark, quint64 junk_mark) {
    if (clean_mark != dbus_uninstall_residual_cleaner_mark_) {
        return;
    }
    Q_EMIT sig_CleanForJunk(Mark(), junk_mark);
}

void ResidualCleaner::on_CleanFinish(QString clean_mark) {
    if (clean_mark != dbus_uninstall_residual_cleaner_mark_) {
        return;
    }
    Q_EMIT sig_CleanFinish(Mark());
}

}
