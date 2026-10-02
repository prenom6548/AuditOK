#include "junk_clean_interface.h"
#include <thread>
#include <QDebug>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QProcess>
#include <QDir>

namespace junk_clean {

static const char * const INSTALLATION_PACKAGE_CLEANER_MARK = "installation_package_cleaner";
static const char * const UNINSTALL_RESIDUAL_CLEANER_MARK = "uninstall_residual_cleaner";
static const char * const SYSTEM_MEMORY_CLEANER_MARK = "system_memory_cleaner";

JunkCleanAdaptor::JunkCleanAdaptor(QObject *obj)
    : QDBusAbstractAdaptor(obj)
{
    qDBusRegisterMetaType<QList<quint64>>();
}

void JunkCleanAdaptor::EmitScanForJunkSignal(QString cleaner_mark,
                                             quint64 junk_mark,
                                             QString path,
                                             quint64 size) const
{
    auto message = QDBusMessage::createSignal(JUNK_CLEAN_DBUS_PATH, JUNK_CLEAN_DBUS_INTERFACE, "ScanForJunk");
    QList<QVariant> args;
    args.push_back(cleaner_mark);
    args.push_back(junk_mark);
    args.push_back(path);
    args.push_back(size);
    message.setArguments(args);
    QDBusConnection::systemBus().send(message);
}

void JunkCleanAdaptor::EmitScanFinishSignal(QString cleaner_mark, quint64 total_size) const {
    auto message = QDBusMessage::createSignal(JUNK_CLEAN_DBUS_PATH, JUNK_CLEAN_DBUS_INTERFACE, "ScanFinish");
    QList<QVariant> args;
    args.push_back(cleaner_mark);
    args.push_back(total_size);
    message.setArguments(args);
    QDBusConnection::systemBus().send(message);
}

void JunkCleanAdaptor::EmitCleanForJunk(QString cleaner_mark, quint64 junk_mark) const {
    auto message = QDBusMessage::createSignal(JUNK_CLEAN_DBUS_PATH, JUNK_CLEAN_DBUS_INTERFACE, "CleanForJunk");
    QList<QVariant> args;
    args.push_back(cleaner_mark);
    args.push_back(junk_mark);
    message.setArguments(args);
    QDBusConnection::systemBus().send(message);
}

void JunkCleanAdaptor::EmitCleanFinish(QString cleaner_mark) const {
    auto message = QDBusMessage::createSignal(JUNK_CLEAN_DBUS_PATH, JUNK_CLEAN_DBUS_INTERFACE, "CleanFinish");
    QList<QVariant> args;
    args.push_back(cleaner_mark);
    message.setArguments(args);
    QDBusConnection::systemBus().send(message);
}

void JunkCleanAdaptor::ScanInstallationPackage() {
    std::thread t([this]() {
        std::lock_guard<std::mutex> mutex(installation_package_mutex_);
        installation_package_junk_.clear();

        quint64 total_size{0};
        quint64 junk_mark{0};
        QDir dir("/var/cache/apt/archives/");
        auto entries_info = dir.entryInfoList(QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);
        for (const auto &info: entries_info) {
            if (info.suffix() != "deb") {
                continue;
            }
            auto path = info.absoluteFilePath();
            auto size = info.size();
            installation_package_junk_.insert(++junk_mark, path);
            total_size += size;
            EmitScanForJunkSignal(INSTALLATION_PACKAGE_CLEANER_MARK, junk_mark, path, size);
        }
        EmitScanFinishSignal(INSTALLATION_PACKAGE_CLEANER_MARK, total_size);
    });
    t.detach();
}

void JunkCleanAdaptor::CleanInstallationPackage(QList<quint64> junk_marks) {
    std::thread t([this, junk_marks]() {
        std::lock_guard<std::mutex> mutex(installation_package_mutex_);

        for (const auto &junk_mark: junk_marks) {
            auto iter = installation_package_junk_.find(junk_mark);
            if (iter == installation_package_junk_.end()) {
                qWarning() << "Install package junk mark[" << junk_mark << "] is not exist.";
                EmitCleanForJunk(INSTALLATION_PACKAGE_CLEANER_MARK, junk_mark);
                continue;
            }
            auto path = iter.value();
            if (!QFile::remove(path)) {
                qCritical() << "delete [" << path << "] junk fail.";
            }
            installation_package_junk_.erase(iter);
            EmitCleanForJunk(INSTALLATION_PACKAGE_CLEANER_MARK, junk_mark);
        }
        EmitCleanFinish(INSTALLATION_PACKAGE_CLEANER_MARK);
    });
    t.detach();
}

void JunkCleanAdaptor::ScanUninstallResidual() {
    static const char * const command = "dpkg -l | grep '^rc' | awk '{print $2}'";

    std::thread t([this]() {
        std::lock_guard<std::mutex> mutex(uninstall_residual_mutex_);
        uninstall_residual_junk_.clear();
        quint64 junk_mark{0};
        quint64 total_size{0};

        QProcess process;
        process.setProgram("bash");
        process.setArguments({"-c", command});
        process.start();
        process.waitForFinished();

        auto output = process.readAllStandardOutput().trimmed();
        auto packages = output.split('\n');
        for (const auto &p: packages) {
            QString package = p.trimmed();
            if (package.isEmpty()) {
                continue;
            }

            // dpkg 无法获取 rc 状态软件包的实际大小，此处先获取安装包正常安装时的大小
            auto get_size_command = QString("dpkg -s %1 | grep Installed-Size | awk '{print $2}'").arg(package);
            QProcess process1;
            process1.setProgram("bash");
            process1.setArguments({"-c", get_size_command});
            process1.start();
            process1.waitForFinished();
            auto output1 = process1.readAllStandardOutput().trimmed();
            bool ok{false};
            auto size = output1.toULongLong(&ok);
            if (!ok) {
                qCritical() << "Uninstall residual cleaner scan get install package size fail.";
            }

            total_size += size;
            uninstall_residual_junk_.insert(++junk_mark, package);
            EmitScanForJunkSignal(UNINSTALL_RESIDUAL_CLEANER_MARK, junk_mark, package, size);
        }
        EmitScanFinishSignal(UNINSTALL_RESIDUAL_CLEANER_MARK, total_size);
    });
    t.detach();
}

void JunkCleanAdaptor::CleanUninstallResidual(QList<quint64> junk_marks) {
    std::thread t([this, junk_marks]() {
        std::lock_guard<std::mutex> mutex(uninstall_residual_mutex_);
        for (const auto &junk_mark: junk_marks) {
            auto iter = uninstall_residual_junk_.find(junk_mark);
            if (iter == uninstall_residual_junk_.end()) {
                qWarning() << "Uninstall residual cleaner clean junk [" << junk_mark << "] is not exist";
                EmitCleanForJunk(UNINSTALL_RESIDUAL_CLEANER_MARK, junk_mark);
                continue;
            }

            auto command = QString("dpkg -P %1").arg(iter.value());
            QProcess process;
            process.setProgram("bash");
            process.setArguments({"-c", command});
            process.start();
            process.waitForFinished();

            uninstall_residual_junk_.erase(iter);
            EmitCleanForJunk(UNINSTALL_RESIDUAL_CLEANER_MARK, junk_mark);
        }
        EmitCleanFinish(UNINSTALL_RESIDUAL_CLEANER_MARK);
    });
    t.detach();
}

void JunkCleanAdaptor::CleanSystemMemory() {
    static const char * const command = "echo 3 > /proc/sys/vm/drop_caches";

    std::thread t([this]() {
        QProcess process;
        process.setProgram("bash");
        process.setArguments({"-c", command});
        process.start();
        process.waitForFinished(-1);
        EmitCleanFinish(SYSTEM_MEMORY_CLEANER_MARK);
    });
    t.detach();
}

}
