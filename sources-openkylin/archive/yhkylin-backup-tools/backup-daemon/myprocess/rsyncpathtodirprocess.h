#ifndef RSYNCPATHTODIRPROCESS_H
#define RSYNCPATHTODIRPROCESS_H

#include <QProcess>

class RsyncPathToDirProcess : public QObject
{
    Q_OBJECT
public:
    explicit RsyncPathToDirProcess(QObject *parent = nullptr);
    virtual ~RsyncPathToDirProcess();

    bool start(QStringList args, bool block = true);

    inline void stop() {
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
    void rsync_finished(int exitCode, QProcess::ExitStatus);
    // m_syncProcess执行结束
    void sync_finished(int exitCode, QProcess::ExitStatus);

private:
    QProcess * m_p;
    QProcess* m_syncProcess;
    int m_currentRate;
    bool m_bSuccess;
    bool m_block;
};

#endif // RSYNCPATHTODIRPROCESS_H
