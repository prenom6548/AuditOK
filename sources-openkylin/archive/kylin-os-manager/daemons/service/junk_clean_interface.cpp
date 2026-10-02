#include "junk_clean_interface.h"
#include <unistd.h>
#include <thread>
#include <QDebug>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include "kom-service.h"
#include "disk_monitor.h"

namespace junk_clean {

static const char * const KYLIN_OS_MANAGER_BOOT_PARTITION_BACKUP_DIR = "/opt/appdata/kylin-os-manager/backup/boot_partition/";
static const char * const INSTALLATION_PACKAGE_CLEANER_MARK = "installation_package_cleaner";
static const char * const UNINSTALL_RESIDUAL_CLEANER_MARK = "uninstall_residual_cleaner";
static const char * const SYSTEM_MEMORY_CLEANER_MARK = "system_memory_cleaner";
static const char * const BOOT_PARTITION_CLEANER_MARK = "boot_partition_cleaner";
static const char * const LOG_CLEANER_MARK = "log_cleaner";

JunkCleanAdaptor::JunkCleanAdaptor(QObject *obj) : QObject(obj)
{
    qDBusRegisterMetaType<QList<quint64>>();

    QDir dir(KYLIN_OS_MANAGER_BOOT_PARTITION_BACKUP_DIR);
    if (!dir.exists()) {
        if (!dir.mkpath(KYLIN_OS_MANAGER_BOOT_PARTITION_BACKUP_DIR)) {
            qCritical() << "Create boot partition backup dir fail";
        }
    }
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
    static const char *const command = "dpkg -l | grep '^rc' | awk '{print $2}'";

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
    static const char *const command = "echo 3 > /proc/sys/vm/drop_caches";

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

/*
 * 1. vmlinuz-*
 * 2. initrd.img-*
 * 3. config-*
 * 4. System.map-*
 * 5. 忽略 dkms 结尾的文件
 */
void JunkCleanAdaptor::ScanBootPartition() {
    std::thread t([this]() {
        std::lock_guard<std::mutex> mutex(boot_partition_mutex_);
        boot_partition_junk_.clear();

        quint64 total_size{0};
        quint64 junk_mark{0};
        const QDir dir("/boot/");
        const auto entries_info = dir.entryInfoList(QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);
        for (const auto &info: entries_info) {
            const auto name = info.fileName();
            if (name.endsWith("dkms")) {
                continue;
            }
            if (name.startsWith("vmlinuz-") ||
                name.startsWith("initrd.img-") ||
                name.startsWith("config-") ||
                name.startsWith("System.map-"))
            {
                const auto path = info.absoluteFilePath();
                const auto size = info.size();
                boot_partition_junk_.insert(++junk_mark, path);
                total_size += size;
                EmitScanForJunkSignal(BOOT_PARTITION_CLEANER_MARK, junk_mark, path, size);
            }
        }
        EmitScanFinishSignal(BOOT_PARTITION_CLEANER_MARK, total_size);
    });
    t.detach();
}

void JunkCleanAdaptor::CleanBootPartition(QList<quint64> junk_marks) {
    std::thread t([this, junk_marks]() {
        std::lock_guard<std::mutex> mutex(boot_partition_mutex_);

        for (const auto junk_mark: junk_marks) {
            auto iter = boot_partition_junk_.find(junk_mark);
            if (iter == boot_partition_junk_.end()) {
                qWarning() << "Boot partition junk mark[" << junk_mark << "] is not exist";
                EmitCleanForJunk(BOOT_PARTITION_CLEANER_MARK, junk_mark);
                continue;
            }

            const auto path = iter.value();
            // 构建新文件名
            QFileInfo file_info(path);
            const auto file_name = file_info.fileName();
            const auto new_name = QString("%1%2").arg(KYLIN_OS_MANAGER_BOOT_PARTITION_BACKUP_DIR, file_name);
            // 移动文件
            if (!QFile::rename(path, new_name)) {
                qCritical() << "Boot partition rename file fail(" << path << "--->" << new_name << ")";
            }
            boot_partition_junk_.erase(iter);
            EmitCleanForJunk(BOOT_PARTITION_CLEANER_MARK, junk_mark);
        }
        EmitCleanFinish(BOOT_PARTITION_CLEANER_MARK);
    });
    t.detach();
}

void JunkCleanAdaptor::ScanLog() {
    std::thread t([this]() {
        std::lock_guard<std::mutex> mutex(log_mutex_);

        log_junk_.clear();
        log_junk_mark_ = 0;
        log_junk_total_size_ = 0;

        ScanSystemLog(QDir("/var/log/"));

        EmitScanFinishSignal(LOG_CLEANER_MARK, log_junk_total_size_);
    });
    t.detach();
}

void JunkCleanAdaptor::ScanSystemLog(const QDir &dir) {
   auto entries = dir.entryInfoList(QDir::Filter::Files | QDir::Filter::Dirs | QDir::NoSymLinks | QDir::Filter::NoDotAndDotDot);
    for (const auto &entry: entries) {
        const auto path = entry.absoluteFilePath();
        if (entry.isFile()) {
            // 只清理文件名中包含 "log" 字段的文件，dmesg 日志也不做清理
            const auto file_name = entry.fileName();
            if (!path.startsWith("/var/log/journal/")) {
                if (!file_name.contains("log") ||
                    file_name.contains("dmesg"))
                {
                    continue;
                }
            }

            const auto size = entry.size();
            if (size == 0) {
                continue;
            }

            log_junk_total_size_ += size;
            log_junk_.insert(++log_junk_mark_, path);

            EmitScanForJunkSignal(LOG_CLEANER_MARK, log_junk_mark_, path, size);
        }
        if (entry.isDir()) {
            if (path == "/var/log/audit" ||
                path == "/var/log/loggie")
            {
                // 不扫描以上目录下的内容
                continue;
            }
            ScanSystemLog(QDir(path));
        }
    }
}

void JunkCleanAdaptor::CleanLog(QList<quint64> junk_marks) {
    std::thread t([this, junk_marks]() {
        std::lock_guard<std::mutex> mutex(log_mutex_);

        for (const auto junk_mark: junk_marks) {
            const auto iter = log_junk_.find(junk_mark);
            if (iter == log_junk_.end()) {
                qWarning() << "Junk clean log cleaner no corresponding logs were found";
                EmitCleanForJunk(LOG_CLEANER_MARK, junk_mark);
                continue;
            }

            const auto path = iter.value();
            QFileInfo file_info(path);
            if (!file_info.exists()) {
                qWarning() << "Junk clean log cleaner file not exists(" << path << ")";
                log_junk_.erase(iter);
                EmitCleanForJunk(LOG_CLEANER_MARK, junk_mark);
                continue;
            }

            const auto file_name = file_info.fileName();
            if (!file_name.contains('.') ||
                file_info.suffix() == "log")
            {
                if (::truncate(path.toStdString().c_str(), 0)) {
                    qCritical() << "Junk clean log cleaner truncate file fail(" << path << ")";
                }
            } else {
                if (!QFile::remove(path)) {
                    qCritical() << "Junk clean log cleaner remove file fail (" << path << ")";
                }
            }

            log_junk_.erase(iter);
            EmitCleanForJunk(LOG_CLEANER_MARK, junk_mark);
        }
        EmitCleanFinish(LOG_CLEANER_MARK);
    });
    t.detach();
}

/*
 * 由于 RecordCleanedFile 函数内修改文件内容，不可重入，故此处不使用异步执行的方式
 * 另外该函数是在清理完成后才会被调用，所以不存在与扫描和清理函数执行冲突的问题
 */
void JunkCleanAdaptor::RecordCleanedFiles(QStringList cleaned_files) {
    monitor::DiskMonitor::RecordCleanedFiles(cleaned_files);
}

}
