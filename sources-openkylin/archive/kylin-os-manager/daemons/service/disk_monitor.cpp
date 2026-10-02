#include "disk_monitor.h"

#include <QDate>
#include <QDebug>
#include <QFileInfo>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QTimer>
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>

namespace {

constexpr char *SIZE_EXCEPTION_FILES_CONFIG_PATH = (char *)"/etc/kylin-os-manager/size_exception_files.conf";
constexpr char *DISK_CAPACITY_EXCEPTION_CONFIG_PATH = (char *)"/etc/kylin-os-manager/disk_capacity_exception.conf";
constexpr quint64 LOG_FILE_TOTAL_SIZE_TOPLIMIT = static_cast<quint64>(10) * 1024 * 1024 * 1024;    // 10G
constexpr quint64 LOG_FILE_SIZE_TOPLIMIT = static_cast<quint64>(300) * 1024 * 1024;    // 300M
constexpr quint64 LOG_FILE_SIZE_INCREASED = static_cast<quint64>(30) * 1024 * 1024;
constexpr char *LOG_EXCEPTION_TIME_FMT = (char *)"yyyy-MM-dd HH:mm:ss";
constexpr char *DBUS_PATH = (char *)"/com/KylinOsManager/DiskMonitor";
constexpr char *DBUS_INTERFACE = (char *)"com.KylinOsManager.DiskMonitor";

}

namespace monitor {

DiskMonitor::DiskMonitor(QObject *parent)
    : QObject(parent),
      log_check_timer_(new QTimer(this)),
      disk_capacity_check_timer_(new QTimer(this))
{

}

DiskMonitor::~DiskMonitor() {

}

void DiskMonitor::on_Run() {
    log_check_timer_->setInterval(10 * 60 * 1000);    // 10 分钟
    connect(log_check_timer_, &QTimer::timeout, this, &DiskMonitor::on_LogFileSizeCheck);
    log_check_timer_->start();

    disk_capacity_check_timer_->setInterval(10 * 60 * 1000);
    connect(disk_capacity_check_timer_, &QTimer::timeout, this, &DiskMonitor::on_DiskCapacityCheck);
    disk_capacity_check_timer_->start();
}

void DiskMonitor::on_LogFileSizeCheck() {
    log_file_size_.clear();

    // 扫描 /var/log/ 目录下的所有文件
    ScanFile(QDir("/var/log"));

    // 扫描所有用户家目录下的日志文件
    QDir dir("/home");
    auto dirs = dir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    for (const auto &d: dirs) {
        auto user_path = QString("/home/%1/.log").arg(d);
        ScanFile(QDir(user_path));
    }

    // 计算总大小
#ifdef ICBC_ENVIRONMENT
    quint64 total_size{0};
    for (auto iter = log_file_size_.cbegin(); iter != log_file_size_.cend(); iter++) {
        total_size += iter.key();
    }
    auto journal_dir_size = DirSize(QDir("/var/log/journal"));
    total_size += journal_dir_size;
    if (total_size > LOG_FILE_TOTAL_SIZE_TOPLIMIT) {
        EmitLogFileTotalSizeToplimit();
        return;
    }
#endif

    QList<QString> exception_files;
    for (auto iter = log_file_size_.cbegin(); iter != log_file_size_.cend(); iter++) {
        if (iter.key() > LOG_FILE_SIZE_TOPLIMIT) {
            quint64 history = log_file_size_history_.value(iter.value());
            quint64 increased = iter.key() - history;
            if (history > 0 && increased > LOG_FILE_SIZE_INCREASED) {
                exception_files.push_back(iter.value());
            }
            log_file_size_history_.insert(iter.value(),iter.key());
        }
    }

    // 判断当天是否有已经清理过的文件再次超出大小限制，同时如果新扫描出的异常文件不再配置文件内，则插入到配置文件中
    do {
        QFile file(SIZE_EXCEPTION_FILES_CONFIG_PATH);
        if (!file.exists()) {
            GenerateSizeExceptionFilesConf(exception_files);
            break;
        }
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Disk monitor load size execption file conf open fail";
            break;
        }
        const auto data = file.readAll();
        file.close();

        // 反序列化
        const auto doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            qCritical() << "Disk monitor unmarshal size exception file conf fail";
            GenerateSizeExceptionFilesConf(exception_files);
            break;
        }
        if (!doc.isObject()) {
            qCritical() << "Disk monitor size exception file conf format error";
            GenerateSizeExceptionFilesConf(exception_files);
            break;
        }

        auto root = doc.object();
        if (!root.contains("date") ||
            !root.contains("exception_files") ||
            !root.contains("exception_count") ||
            !root.contains("log_exception_time"))
        {
            qCritical() << "Disk monitor size exception file conf field miss";
            GenerateSizeExceptionFilesConf(exception_files);
            break;
        }

        // 判断是否是新的一天的扫描结果
        const auto date = QDate::fromString(root.value("date").toString(), "yyyy-MM-dd");
        const auto current_date = QDate::currentDate();
        if (current_date != date) {
            GenerateSizeExceptionFilesConf(exception_files);
            break;
        }

        // 当天的扫描结果，比对更新配置文件
        if (!root.value("exception_files").isObject()) {
            qCritical() << "Disk monitor size exception file conf exception_files field format error";
            GenerateSizeExceptionFilesConf(exception_files);
            break;
        }

        bool ok{false};
        auto exception_files_obj = root.value("exception_files").toObject();
        for (const auto &exception_file: exception_files) {
            if (exception_files_obj.contains(exception_file)) {
                if (exception_files_obj.value(exception_file).toBool()) {
                    ok = true;
                }
            } else {
                exception_files_obj.insert(exception_file, false);
            }
        }

        // 判断距离上次弹 400 弹窗是否已经超过 24 小时，24 小时内只弹一次
        auto curr_date_time  = QDateTime::currentDateTime();
        auto saved_date_time = QDateTime::fromString(root.value("log_exception_time").toString(), LOG_EXCEPTION_TIME_FMT);
        auto diff_hours      = saved_date_time.secsTo(curr_date_time) / 3600;
        if (ok && diff_hours >= 24) {
            root["log_exception_time"] = curr_date_time.toString(LOG_EXCEPTION_TIME_FMT);
        }

        // 更新配置文件
        root["exception_files"] = exception_files_obj;
        QJsonDocument modify_doc;
        modify_doc.setObject(root);
        const auto modify_data = modify_doc.toJson();

        QFile modify_file(SIZE_EXCEPTION_FILES_CONFIG_PATH);
        if (modify_file.open(QIODevice::WriteOnly)) {
            const auto size = modify_file.write(modify_data);
            if (size != modify_data.size()) {
                qWarning() << "Disk monitor update size exception file conf write fail";
            }
            modify_file.close();
        }

        if (ok && diff_hours >= 24) {
            EmitLogFileException();
            return;
        }
    } while (false);

    // 判断是否有文件超过 300M
    if (!exception_files.isEmpty()) {
        IncrementLogExceptionCount();
        EmitLogFileSizeToplimit();
    }
}

void DiskMonitor::on_DiskCapacityCheck() {
    QFileInfo file_info(DISK_CAPACITY_EXCEPTION_CONFIG_PATH);
    if (!file_info.exists()) {
        GenerateDiskCapacityExceptionConf();
    }

    QProcess process;
    process.setProgram("df");
    process.setArguments({"-h"});
    process.start();
    process.waitForFinished();

    auto output = process.readAllStandardOutput();
    auto lines = QString::fromUtf8(output).split('\n');
    for (const auto &line: lines) {
        auto elementes = line.split(" ");
        elementes.removeAll("");
        if (elementes.size() != 6) {
            qCritical() << "Disk capacity check element count error: " << line << " [" << elementes << "]";
            continue;
        }

        auto mount_point = elementes.at(5);
        if (mount_point != "/" &&
            mount_point != "/data")
        {
            continue;
        }

        bool ok{false};
        auto used = elementes.at(4).trimmed().remove("%");
        auto percent = used.toDouble(&ok);
        if (!ok) {
            qCritical() << "Convert " <<  mount_point << "mount point disk usage fail.";
            continue;
        }
        // 当使用率超过 90% 时，发出信号
        if (percent > 90.0) {
            UpdateDiskCapacityExceptionCount();
            EmitDiskUsageRateToplimit();
            return;
        }
    }
}

void DiskMonitor::ScanFile(const QDir &dir) {
    if (!dir.exists()) {
        return;
    }

    auto files = dir.entryInfoList(QDir::Filter::Files | QDir::Filter::NoDotAndDotDot | QDir::Filter::NoSymLinks);
    for (const auto &f: files) {
        // 只检测文件名中包含 "log" 字段的文件，dmesg 日志也不做清理
        const auto file_name = f.fileName();
        if (!file_name.contains("log") ||
            file_name.contains("dmesg"))
        {
            continue;
        }

        auto path = f.absoluteFilePath();
        auto size = f.size();
        log_file_size_.insert(size, path);
    }

    auto dirs = dir.entryInfoList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::NoSymLinks);
    for (const auto &d: dirs) {
        auto path = d.filePath();
        if (path == "/var/log/audit" ||
#ifdef ICBC_ENVIRONMENT
            path == "/var/log/journal" ||
#endif
            path == "/var/log/loggie")
        {
            // 不扫描以上目录下的内容
            continue;
        }
        ScanFile(QDir(path));
    }
}

quint64 DiskMonitor::DirSize(const QDir &dir) {
    if (!dir.exists()) {
        return 0;
    }

    quint64 size{0};
    auto files = dir.entryInfoList(QDir::Filter::Files | QDir::Filter::NoDotAndDotDot | QDir::Filter::NoSymLinks);
    for (const auto &f: files) {
        size += f.size();
    }

    auto dirs = dir.entryInfoList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot | QDir::Filter::NoSymLinks);
    for (const auto &d: dirs) {
        auto path = d.filePath();
        size += DirSize(QDir(path));
    }

    return size;
}

void DiskMonitor::EmitLogFileTotalSizeToplimit() {
    auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "LogFileTotalSizeToplimit");
    QDBusConnection::systemBus().send(message);
}

void DiskMonitor::EmitLogFileSizeToplimit() {
    auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "LogFileSizeToplimit");
    QDBusConnection::systemBus().send(message);
}

void DiskMonitor::EmitDiskUsageRateToplimit() {
    auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "DiskUsageRateToplimit");
    QDBusConnection::systemBus().send(message);
}

void DiskMonitor::EmitLogFileException() {
    auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "LogFileExecption");
    QDBusConnection::systemBus().send(message);
}

/*
 * {
 *     date: 2024-10-10
 *     exception_count: 0
 *     exception_files: {
 *         # 文件路径      今天是否清理过
 *         /xxx/xxx/xxx: flase
 *     }
 * }
 */
void DiskMonitor::GenerateSizeExceptionFilesConf(const QList<QString> &exception_files) {
    QJsonObject root;

    // 插入当前日期
    const auto current_date = QDate::currentDate().toString("yyyy-MM-dd");
    root.insert("date", current_date);

    // 插入400异常弹框时间
    root.insert("log_exception_time", QDateTime::fromTime_t(0).toString(LOG_EXCEPTION_TIME_FMT));

    // 插入异常扫描次数
    root.insert("exception_count", 0);

    // 插入异常文件
    QJsonObject exception_files_obj;
    for (const auto &execption_file: exception_files) {
        exception_files_obj.insert(execption_file, false);
    }
    root.insert("exception_files", exception_files_obj);

    // 序列化
    QJsonDocument doc;
    doc.setObject(root);
    const auto data = doc.toJson();

    // 写入配置文件
    QFile file(SIZE_EXCEPTION_FILES_CONFIG_PATH);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Disk monitor open size exception files conf fail";
        return;
    }
    if (!file.setPermissions(QFileDevice::Permission::ReadOwner |
                             QFileDevice::Permission::WriteOwner |
                             QFileDevice::Permission::ReadGroup |
                             QFileDevice::Permission::ReadOther))
    {
        qWarning() << "Disk monitor set size exception files conf permission fail";
    }

    const auto size = file.write(data);
    if (size != data.size()) {
        qWarning() << "Disk monitor write size exception files conf incomplete";
    }
    file.close();
}

void DiskMonitor::IncrementLogExceptionCount() const {
    QFile file(SIZE_EXCEPTION_FILES_CONFIG_PATH);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Disk monitor increment log exception count open file fail";
        return;
    }
    const auto data = file.readAll();
    file.close();

    // 反序列化
    const auto doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qCritical() << "Disk monitor increment log exception count unmashal fail";
        return;
    }
    if (!doc.isObject()) {
        qCritical() << "Disk monitor increment log exception count json format error";
        return;
    }

    // 自增异常次数
    auto root = doc.object();
    if (!root.contains("exception_count")) {
        qCritical() << "Disk monitro increment log exception count exception_count field miss";
        return;
    }
    auto count = root.value("exception_count").toInt();
    root["exception_count"] = ++count;

    // 修改扫描间隔
    if (count >= 3) {
        log_check_timer_->setInterval(30 * 60 * 1000);
    } else {
        log_check_timer_->setInterval(10 * 60 * 1000);
    }

    // 序列化
    QJsonDocument modify_doc;
    modify_doc.setObject(root);
    const auto modify_data = modify_doc.toJson();

    // 写入文件
    QFile modify_file(SIZE_EXCEPTION_FILES_CONFIG_PATH);
    if (!modify_file.open(QIODevice::WriteOnly)) {
        qCritical() << "Disk monitor increment log exception count write open file fail";
        return;
    }
    const auto size = modify_file.write(modify_data);
    if (size != modify_data.size()) {
        qWarning() << "Disk monitor increment log exception count write size exception";
    }
    modify_file.close();
}

/*
 * {
 *     date: 2024-10-16
 *     exception_count: 0
 * }
 */
void DiskMonitor::GenerateDiskCapacityExceptionConf() {
    QJsonObject root;

    // 插入当前时间
    const auto current_date = QDate::currentDate().toString("yyyy-MM-dd");
    root.insert("date", current_date);

    // 插入异常次数
    root.insert("exception_count", 0);

    // 序列化
    QJsonDocument doc;
    doc.setObject(root);
    const auto data = doc.toJson();

    // 写入文件
    QFile file(DISK_CAPACITY_EXCEPTION_CONFIG_PATH);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Disk monitor generate disk capacity exception conf open file fail";
        return;
    }
    if (!file.setPermissions(QFileDevice::Permission::ReadOwner |
                             QFileDevice::Permission::WriteOwner |
                             QFileDevice::Permission::ReadGroup |
                             QFileDevice::Permission::ReadOther))
    {
        qWarning() << "Disk monitor generate disk capacity exception conf set permission fail";
    }
    const auto size = file.write(data);
    if (size != data.size()) {
        qWarning() << "Disk monitor generate disk capacity exception conf write size incomplete";
    }
    file.close();
}

void DiskMonitor::UpdateDiskCapacityExceptionCount() {
    // 读取文件内容
    QFile file(DISK_CAPACITY_EXCEPTION_CONFIG_PATH);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Disk monitor update disk capacity exception count open file fail";
        return;
    }
    const auto data = file.readAll();
    file.close();

    // 反序列化
    const auto doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qCritical() << "Disk monitor update disk capacity exception count unmashal json fail";
        GenerateDiskCapacityExceptionConf();
        return;
    }
    if (!doc.isObject()) {
        qCritical() << "Disk monitor update disk capacity exception count json format error";
        GenerateDiskCapacityExceptionConf();
        return;
    }

    auto root = doc.object();
    if (!root.contains("date") ||
        !root.contains("exception_count"))
    {
        qCritical() << "Disk monitor update disk capacity exception count json field miss";
        GenerateDiskCapacityExceptionConf();
        return;
    }

    // 比较日期
    const auto date = QDate::fromString(root.value("date").toString(), "yyyy-MM-dd");
    const auto current_date = QDate::currentDate();
    if (date != current_date) {
        root["date"] = current_date.toString("yyyy-MM-dd");
        root["exception_count"] = 1;
        disk_capacity_check_timer_->setInterval(10 * 60 * 1000);
    } else {
        auto count = root.value("exception_count").toInt();
        root["exception_count"] = ++count;

        if (count >= 3) {
            disk_capacity_check_timer_->setInterval(30 * 60 * 1000);
        } else {
            disk_capacity_check_timer_->setInterval(10 * 60 * 1000);
        }
    }

    // 序列化
    QJsonDocument modify_doc;
    modify_doc.setObject(root);
    const auto modify_data = modify_doc.toJson();

    // 写入文件
    QFile modify_file(DISK_CAPACITY_EXCEPTION_CONFIG_PATH);
    if (!modify_file.open(QIODevice::WriteOnly)) {
        qCritical() << "Disk monitor update disk capacity exception count write open file fail";
        return;
    }
    const auto size = modify_file.write(modify_data);
    if (size != modify_data.size()) {
        qWarning() << "Disk monitor update disk capacity exception count write file incomplete";
    }
    modify_file.close();
}

void DiskMonitor::RecordCleanedFiles(QList<QString> cleaned_files) {
    // 读取配置文件
    QFile file(SIZE_EXCEPTION_FILES_CONFIG_PATH);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Disk monitor record cleaned files open conf file fail";
        return;
    }
    const auto data = file.readAll();
    file.close();

    // 反序列化
    const auto doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qCritical() << "Disk monitor record cleaned files unmarshal conf fail";
        return;
    }
    if (!doc.isObject()) {
        qCritical() << "Disk monitor record cleaned files conf file format error";
        return;
    }

    auto root = doc.object();
    if (!root.contains("date") ||
        !root.contains("exception_files"))
    {
        qCritical() << "Disk monitor record cleaned files conf file field miss";
        return;
    }

    // 判断配置文件中记录的是否为当日的数据
    const auto date = QDate::fromString(root.value("date").toString(), "yyyy-MM-dd");
    const auto current_date = QDate::currentDate();
    if (date != current_date) {
        return;
    }

    if (!root.value("exception_files").isObject()) {
        qCritical() << "Disk monitor record cleaned files conf file execption_files field format error";
        return;
    }

    // 更新数据
    auto exception_files_obj = root.value("exception_files").toObject();
    for (const auto &junk: cleaned_files) {
        if (exception_files_obj.contains(junk)) {
            exception_files_obj[junk] = true;
        }
    }

    // 写入文件
    root["exception_files"] = exception_files_obj;
    QJsonDocument modify_doc;
    modify_doc.setObject(root);
    const auto modify_data = modify_doc.toJson();

    QFile modify_file(SIZE_EXCEPTION_FILES_CONFIG_PATH);
    if (!modify_file.open(QIODevice::WriteOnly)) {
        qCritical() << "Disk monitor record cleaned files write conf file open fail";
        return;
    }
    const auto size = modify_file.write(modify_data);
    if (size != modify_data.size()) {
        qWarning() << "Disk monitor record cleaned files write conf file incomplete";
    }
    modify_file.close();
}

}
