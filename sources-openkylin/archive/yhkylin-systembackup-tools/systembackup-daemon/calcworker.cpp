#include "calcworker.h"
#include "../common/utils.h"
#include <QProcess>
#include <QTime>

CalcWorker::CalcWorker(QObject *parent)
    : QObject{parent}
{}

void CalcWorker::doWork()
{
    while(true) {
        QString sysPath = "/ostree/pkgs/ovl-";
        qint64 size=0;

        // get ostree commit id
        QStringList args;
        QString otherArgs = " awk -F ' ' '{print $3}' ";
        args << "admin";
        args << "status";
        args << "|";
        args << "grep '*'";
        args << "|";
        args << otherArgs;

        QProcess process;
        // process.setProgram("ostree");
        QString fullCommand = "ostree " + args.join(" ");


        process.start("bash", QStringList() << "-c" << "ostree admin status | grep '*' | awk '{print $3}'");
        process.waitForFinished(-1);

        if (process.exitCode() != 0) {
            qDebug() << "Error:" << process.readAllStandardError();
            return ;
        }
        QString output = process.readAllStandardOutput().trimmed();
        if (output.isEmpty()) {
            qDebug() << "No hidden files found or command failed.";
            return ;
        }else{
            sysPath += output;
            size = Utils::getDirectorySize(sysPath);
        }
//        qDebug()<<"ostree syspath size ## "<< size;

        qint64 tmpSize = calcSizeForHome();
        if(tmpSize>0){
            size += tmpSize;
        }

        // 备份数据大小
        qint64 itotalSize = size;
//        qDebug() << "itotalSize: " << itotalSize;

        // 备份点所在路径/data/下剩余空间大小
        qint64 leftSize = Utils::getLeftSize("/data/");
//        qDebug() << "leftSize: " << leftSize;

        // 将备份数据大小和剩余空间大小保存到文件
        Utils::saveSizeToFile(itotalSize, leftSize);

        // 每隔30秒更新一次数据
        Utils::wait(30);
    }
}

qint64 CalcWorker::calcSizeForHome()
{
    QString homePath = "/home/" + g_userName;
    QString command = QString("du -sb %1/.[^.]* | awk '{sum+=$1} END {print sum}'").arg(homePath);

    // 使用QProcess执行命令
    QProcess process;
    process.start("bash", QStringList() << "-c" << command);
    process.waitForFinished();

    // 检查命令是否执行成功
    if (process.exitCode() != 0) {
        qDebug() << "Error:" << process.readAllStandardError();
        return -1;
    }

    // 获取输出并转换为 qint64
    QByteArray output = process.readAllStandardOutput().trimmed();
    if (output.isEmpty()) {
        qDebug() << "No hidden files found or command failed.";
        return -1;
    }

    bool ok;
    qint64 totalBytes = output.toLongLong(&ok);  // 转换为 qint64

    if (!ok) {
        qDebug() << "Failed to parse output as integer.";
        return -1;
    }

    // 输出结果（qint64）
//    qDebug() << "Total hidden files size (bytes):" << totalBytes;
    return totalBytes;
}
