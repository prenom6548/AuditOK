#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#include <QProcess>

class SubProcess : public QObject
{
    Q_OBJECT
public:
    explicit SubProcess(QObject *parent = nullptr);
    virtual ~SubProcess();

    void setWorkingDirectory(const QString& workDir) {
        m_p->setWorkingDirectory(workDir);
    }
    QString workingDirectory() const{
        return m_p->workingDirectory();
    }

    bool start(const QString& exe, const QStringList& args = {}, bool block = false);

    QString readOutput() {
        // 仅阻塞执行时用；非阻塞时请监控output获取结果
        return m_output;
    }

    inline void stop() {
        m_p->kill();
    }

signals:
    // 结果信号
    void finished(bool result);
    // 结果输出
    void output(QString msg);

private slots:
    // m_p执行结束
    void processFinished(int exitCode, QProcess::ExitStatus);

private:
    QProcess * m_p;
    bool m_block = false;
    QString m_output;    // 仅阻塞调用外部程序时使用此数据成员保存终端输出结果。
};

#endif // SUBPROCESS_H
