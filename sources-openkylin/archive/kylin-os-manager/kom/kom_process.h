/*
 * 封装 v11 运行进程的方法，提供与 QProcess 一致的接口
 */

#ifndef KYLIN_OS_MANAGER_KOM_KOM_PROCESS_H
#define KYLIN_OS_MANAGER_KOM_KOM_PROCESS_H

#include <QObject>
#include <QProcess>
#include <QByteArray>

namespace kom {

class KomProcess: public QObject {
    Q_OBJECT
public:
    KomProcess(QObject *parent = nullptr);
    void start(const QString &program, const QStringList &arguments = {});
    void close();
    QByteArray readAllStandardOutput();
    QByteArray readAllStandardError();

Q_SIGNALS:
    void started();
    void errorOccurred(QProcess::ProcessError error);
    void readyReadStandardOutput();
    void readyReadStandardError();
    void finished(int exitCode, QProcess::ExitStatus exitStatus = QProcess::NormalExit);
};

}

#endif
