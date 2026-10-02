#ifndef CALCBACKUPSIZE_H
#define CALCBACKUPSIZE_H

#include <QStringList>
#include <QProcess>

#define GB (1024 * 1024 * 1024)
#define MB (1024 * 1024)
#define KB (1024)

class CalcBackupSize : public QObject
{
    Q_OBJECT
public:
    CalcBackupSize(QObject* parent = nullptr);
    ~CalcBackupSize();

    /**
     * @brief 计算备份大小，单位字节
     * @param args rsync参数列表
     * @param block 是否阻塞计算模式。默认true——阻塞计算模式
     * @return block为true时返回值为待备份数据大小，单位字节；block为false时，默认返回0
     */
    qint64 start(QStringList args, bool block = true);

    void stop() { m_process->kill(); }

signals:
    // 计算结束信号
    void finished(qint64 size);

private slots:
    // process结束槽
    void processFinish(int exitCode, QProcess::ExitStatus);

private:
    /**
     * @brief 解析process的输出结果
     */
    void parseResult();

    QProcess * m_process;

    qint64 m_size = 0; // 备份大小，单位字节
};

#endif // CALCBACKUPSIZE_H
