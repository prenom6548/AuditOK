#include "filemd5.h"
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QApplication>
#include <QDebug>

FileMd5::FileMd5(QObject *parent) : QObject(parent)
{

}

void FileMd5::slotGetFileInfo(const QString &path)
{
    isCanceled = false;
    // 获取目录的文件列表 获取文件md5值 返回结果
    QHash<QByteArray,QStringList> ret;
    m_fileinfo.clear();

    QStringList files = getFiles(path);

    int value = 0;
    for (QHash<long, QStringList>::iterator it = m_fileinfo.begin(); it != m_fileinfo.end(); ++it) {
        value += it.value().count();
        if (isCanceled) {
            return;
        }
        if (it.value().count() > 1) {
            for(int i = 0; i < it.value().count(); i++) {
                QString fileName = it.value().at(i);
                QByteArray md5 = getFileMd5(fileName);
                ret[md5].append(fileName);
                if (isCanceled) {
                    return;
                }
            }
        } else {
            emit signalProgressValue(value, files.count());
            continue;
        }
        if (isCanceled) {
            return;
        }
        emit signalProgressValue(value, files.count());
        emit signalFileInfo(ret);
        ret.clear();
    }
    // emit signalFileInfo(ret);
    // for(int i = 0; i < files.count(); i++){
    //     if (isCanceled) {
    //         break;
    //     }
    //     QString fileName = files.at(i);
    //     QByteArray md5 = getFileMd5(fileName);

    //     ret[md5].append(fileName);
    //     emit signalProgressValue(i+1, files.count());
    // }
    // if (isCanceled) {
    //     return;
    // }
    // emit signalFileInfo(ret);
}

void FileMd5::slotCannelGetMd5()
{
    isCanceled = true;
}

QStringList FileMd5::getFiles(const QString &path)
{
    QStringList ret;

    QDir dir(path);
    // .表示当前目录，..表示上一级目录
    // entryInfoList：返回目录中所有文件和目录的QFileInfo对象列表
    QFileInfoList infoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for(int i=0; i < infoList.count(); i++){
        if (isCanceled) {
            break;
        }
        QFileInfo info = infoList.at(i);
        if(info.isDir()){
            QString subDir = info.absoluteFilePath();
            QStringList files = getFiles(subDir);
            ret.append(files);
        }else{
            QString fileName = info.absoluteFilePath();
            ret.append(fileName);
            m_fileinfo[info.size()].append(fileName);
        }
    }
    return ret;
}

QByteArray FileMd5::getFileMd5(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)){
        return QByteArray();
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    while(!file.atEnd()){

        if (isCanceled) {
            break;
        }
        QByteArray content = file.read(100 * 1024 * 1024); //一次读取100MB的内容
        hash.addData(content); //将content的数据添加到密码散列
        qApp->processEvents(); //防止整个程序卡住（作用不大）
    }
    QByteArray md5 = hash.result().toHex(); //获取最终的哈希值
    file.close();
    return md5;
}

