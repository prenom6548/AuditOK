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
    *m_cancel = false;
    *m_sameFileNum = 0;
    // 获取目录的文件列表 获取文件md5值 返回结果
    QHash<QByteArray,QStringList> ret;
    QHash<QByteArray,QStringList> tmpmd5;
    m_fileinfo.clear(); // m_fileinfo存放文件大小和文件路径
    ret.clear();

    QStringList files = getFiles(path); // 获取全部的文件列表

    int value = 0;
    if (files.count() == 0) {
        emit signalProgressValue(100, 100);
        return;
    } else {
        emit signalProgressValue(value, files.count() + 5);
    }
    int num = 0;
    for (QHash<long, QStringList>::iterator it = m_fileinfo.begin(); it != m_fileinfo.end(); ++it) {
        value += it.value().count();
        if (*m_cancel) {
            return;
        }
        if (it.key() > 0 && it.value().count() > 1) {
            tmpmd5 = filtr(it.key(),it.value());
            if (*m_cancel) {
                return;
            }

            // 遍历MD5哈希表，含有相同MD5的文件
            for (QHash<QByteArray, QStringList>::iterator it = tmpmd5.begin(); it != tmpmd5.end(); ++it) {
                if (*m_cancel) {
                    return;
                }
                if (it.value().count() > 1 && it.key() != "") {
                    ret[it.key()].append(it.value());
                    num += it.value().count();
                    *m_sameFileNum = num;
                    // qDebug() << " really file num : " << num ;
                }
            } 
        } else {
            // emit signalProgressValue(value, files.count());
            emit signalProgressValue(value, files.count() + 5);
            continue;
        }
        if (*m_cancel) {
            return;
        }
        // emit signalProgressValue(value, files.count());
        emit signalProgressValue(value, files.count() + 5);
    }
    emit signalFileInfo(ret);
    qDebug() << " really file num : " << num ;
}

bool *FileMd5::slotGetCancelHandle()
{
    if (m_cancel == nullptr) {
        m_cancel = new bool;
        *m_cancel = false;
    }
    return m_cancel;
}

int *FileMd5::slotGetSameFileNum()
{
    if (m_sameFileNum == nullptr) {
        m_sameFileNum = new int;
        *m_sameFileNum = 0;
    }
    return m_sameFileNum;
}

QStringList FileMd5::getFiles(const QString &path)
{
    QStringList ret;

    QDir dir(path);
    // .表示当前目录，..表示上一级目录
    // entryInfoList：返回目录中所有文件和目录的QFileInfo对象列表
    QFileInfoList infoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for(int i=0; i < infoList.count(); i++){
        if (*m_cancel) {
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

        if (*m_cancel) {
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


QHash<QByteArray, QStringList> FileMd5::filtr(long size ,const QStringList &list)
{
    QHash<QByteArray, QStringList> ret;
    ret.clear();
    // 当文件大小小于128字节的文件时，直接计算md5值
    if (size < 128) {
        for(int i = 0; i < list.count(); i++) {
            QString fileName = list.at(i);
            QByteArray md5 = getFileMd5(fileName);
            ret[md5].append(fileName);
            if (*m_cancel) {
                return ret;
            }
        }
        return ret;
    }

    QHash<QByteArray, QStringList> tmp;
    // 读取文件部分数据，存放到哈希表中，key为文件16字节数据，value为文件路径的集合列表
    for(int i = 0; i < list.count(); i++) {
        QString fileName = list.at(i);
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly)){
            continue;
        }
        int seek = size/3;
        file.seek(seek);
        QByteArray type = file.read(8);
        seek = seek *2;
        file.seek(seek);
        type += file.read(8);
        tmp[type].append(fileName);
        file.close();
        if (*m_cancel) {
            return ret;
        }
    }

    // 遍历读取文件数据的哈希表，计算md5值
    for (const QStringList &tmplist : tmp.values()) {
        if (tmplist.length() < 2) {
            continue;
        }
        if (*m_cancel) {
            return ret;
        }
        for (const QString &fileName : tmplist) {
            QByteArray md5 = getFileMd5(fileName);
            ret[md5].append(fileName);
            if (*m_cancel) {
                return ret;
            }
        }
    }
    return ret;
}
