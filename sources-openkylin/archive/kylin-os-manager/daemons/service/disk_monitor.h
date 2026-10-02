#include <QObject>
#include <QMultiMap>
#include <QString>
#include <QDir>
#include <QTimer>
#include <QList>

/*
 * 异常触发条件：
 *     1. 单个文件超过 300M且和上次扫描相比新增超过30M (前 3 次弹气泡弹窗, 3 次后弹窗口)
 *     2. 总文件大小超过 10G (只在工行环境生效)
 *     3. 当天内，已经清理过的文件，再次超过单文件大小上限 (弹 400 异常窗口, 且该窗口 24 小时只弹一次)
 *     4. 磁盘容量不足 20%
 *
 * 会发出的信号：
 *     signal: LogFileTotalSizeToplimit    文件的总大小超过了限制
 *     signal: LogFileSizeToplimit         单个日志文件大小超过 300M且和上次扫描相比新增超过30M
 *     signal: LogFileException            当天内已经清理过的文件又超出了大小限制，弹 400 弹窗
 *     signal: DiskUsageRateToplimit       磁盘容量不足 20%
 */

namespace monitor {

class DiskMonitor: public QObject {
    Q_OBJECT
public:
    DiskMonitor(QObject *parent = nullptr);
    ~DiskMonitor();
    static void RecordCleanedFiles(QList<QString> cleaned_files);

public Q_SLOTS:
    void on_Run();

private Q_SLOTS:
    void on_LogFileSizeCheck();
    void on_DiskCapacityCheck();

private:
    void ScanFile(const QDir &dir);
    quint64 DirSize(const QDir &dir);
    void GenerateSizeExceptionFilesConf(const QList<QString> &exception_files);
    void IncrementLogExceptionCount() const;
    void GenerateDiskCapacityExceptionConf();
    void UpdateDiskCapacityExceptionCount();
    void EmitLogFileTotalSizeToplimit();
    void EmitLogFileSizeToplimit();
    void EmitLogFileException();
    void EmitDiskUsageRateToplimit();

    QTimer *                    log_check_timer_;
    QTimer *                    disk_capacity_check_timer_;
    QMultiMap<quint64, QString> log_file_size_;
    QMap<QString, quint64> log_file_size_history_;
};

}
