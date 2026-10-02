#include <QObject>
#include <QMultiMap>
#include <QString>
#include <QDir>
#include <QTimer>

/*
 * signal: LogFileTotalSizeToplimit
 * signal: LogFileSizeToplimit
 * signal: LogFileException
 * signal: DiskUsageRateToplimit
 */

namespace monitor {

class DiskMonitor: public QObject {
    Q_OBJECT
public:
    DiskMonitor(QObject *parent = nullptr);
    ~DiskMonitor();

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
};

}
