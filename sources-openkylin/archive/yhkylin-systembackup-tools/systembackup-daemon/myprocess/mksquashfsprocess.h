#ifndef MKSQUASHFSPROCESS_H
#define MKSQUASHFSPROCESS_H

#include <QProcess>
#include <QObject>

class MkSquashFSProcess : public QObject
{
    Q_OBJECT
public:
    explicit MkSquashFSProcess(QObject *parent = nullptr);
    virtual ~MkSquashFSProcess();

    bool start(const QStringList &args);

    void stop() {
        m_p->kill();
        m_syncProcess->kill();
    }

signals:
    // 结果信号
    void finished(bool result);
    // 进度百分比
    void progress(int currentRate);

private slots:
    // m_p执行结束
    void mksquashfs_finished(int exitCode, QProcess::ExitStatus);
    // m_syncProcess执行结束
    void sync_finished(int exitCode, QProcess::ExitStatus);

private:
    QProcess * m_p;
    QProcess* m_syncProcess;
    int m_currentRate;
    bool m_bSuccess;
};

#endif // MKSQUASHFSPROCESS_H
