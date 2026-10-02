#include "fileparser.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDir>
#include <QUrl>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

const QStringList WECHAT_FILE_LIST = {"wechatVideo", "wechatMusic", "wechatDocument", "wechatPicture"};
const QStringList QQ_FILE_LIST = {"qqVideo", "qqMusic", "qqDocument", "qqPicture"};
const QString QQ_KEY = "qq";
const QString WECHAT_KEY = "wechat";
const QString LINE_SEP = "\n";
const QString COL_SEP = ":";
const QString IS_UPDATED_TREE = "1";
const QString IS_UPDATED_FALSE = "0";

FileParSer::FileParSer(QUrl url, QObject *parent) : QObject(parent)
{
    m_url = url;
}
FileParSer::~FileParSer() {}

void FileParSer::getItemCountList(QString path, FileParSer::KeyType type, QMap<int, int> &map)
{
    map.clear();
    QFile *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        // 打开文件失败
        qCritical() << "fail to open file" << path;
        delete file;
        file = nullptr;
        return;
    }
    QByteArray data = file->readAll();
    QString str = data;
    QStringList strList = str.split(LINE_SEP);
    int qqFileNum = 0;
    int wechatFileNum = 0;
    for (int i = 0; i < strList.count(); ++i) {
        QString info = strList.value(i);
        QStringList infoList = info.split(COL_SEP);
        QString key = infoList.value(KEY_INDEX::KEYNAME);
        QString fileNum = infoList.value(KEY_INDEX::NUM);
        int num = 0;
        if (!fileNum.isEmpty()) {
            num = fileNum.toInt();
        }
        if (!key.isEmpty()) {
            if (key == FILENUM_KEY_LIST.value(int(type) + int(ItemType::PictureNum))) {
                map.insert(ItemType::PictureNum, num);
            } else if (key == FILENUM_KEY_LIST.value(int(type) + int(ItemType::VideoNum))) {
                map.insert(ItemType::VideoNum, num);
            } else if (key == FILENUM_KEY_LIST.value(int(type) + int(ItemType::MusicNum))) {
                map.insert(ItemType::MusicNum, num);
            } else if (key == FILENUM_KEY_LIST.value(int(type) + int(ItemType::DocNum))) {
                map.insert(ItemType::DocNum, num);
            }
        }
        if (WECHAT_FILE_LIST.contains(key)) {
            // 累计微信文件数量
            wechatFileNum += num;
        }
        if (QQ_FILE_LIST.contains(key)) {
            // 累计QQ文件数量
            qqFileNum += num;
        }
    }
    if (type == KeyType::All) {
        map.insert(ItemType::WeChatNum, wechatFileNum);
        map.insert(ItemType::QQNum, qqFileNum);
    }
    file->close();
    file->deleteLater();
    file = nullptr;
    return;
}

void FileParSer::getFileInfoList(QString path, QList<FileInfo> &list, QString flag)
{
    list.clear();
    QFile *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        // 打开文件失败
        qCritical() << "fail to open file" << path;
        delete file;
        file = nullptr;
        return;
    }

    QByteArray data = file->readAll();
    QString strInfoAll = data;
    if (strInfoAll.isEmpty()) {
        qInfo() << path << " isEmpty";
        return;
    }
    QByteArray infoJsonBuf = strInfoAll.toUtf8();
    QJsonDocument infoJsonDoc = QJsonDocument::fromJson(infoJsonBuf);
    QJsonArray infoArray = infoJsonDoc.array();
    for (int i = 0; i < infoArray.size(); i++) {
        QJsonObject infoObj = infoArray.at(i).toObject();
        FileInfo fileInfo;
        fileInfo.setId(infoObj.value("id").toInt());
        fileInfo.setName(infoObj.value("name").toString());
        QString dateTime = infoObj.value("date").toString();
        fileInfo.setDateTime(dateTime);
        fileInfo.setDate(dateTime.mid(0, dateTime.lastIndexOf(" ")));
        double size = infoObj.value("size").toDouble();
        QString sizeStr = QString::number(size, 'f', 0);
        fileInfo.setSize(sizeStr.toLongLong());
        fileInfo.setPath(infoObj.value("path").toString());
        QString thumbPath = infoObj.value("thumbPath").toString();
        if (!thumbPath.isEmpty()) {
            // 获取缩略图路径
            fileInfo.setThumbnailPath(path.mid(0, path.lastIndexOf("/")) + "/thumbnail/" + flag
                                      + thumbPath.mid(thumbPath.lastIndexOf("/") + 1, thumbPath.size()));
        }
        list.append(fileInfo);
    }
    qInfo() << "In total, " << list.size() << " pieces of information were obtained.";
    file->close();
    file->deleteLater();
    file = nullptr;

    return;
}

void FileParSer::getThumbList(QString path, QStringList &list)
{
    list.clear();
    QFile *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        // 打开文件失败
        qCritical() << "fail to open file" << path;
        delete file;
        file = nullptr;
        return;
    }

    QByteArray data = file->readAll();
    QString strInfoAll = data;
    if (strInfoAll.isEmpty()) {
        qInfo() << path << " isEmpty";
        return;
    }
    QByteArray infoJsonBuf = strInfoAll.toUtf8();
    QJsonDocument infoJsonDoc = QJsonDocument::fromJson(infoJsonBuf);
    QJsonArray infoArray = infoJsonDoc.array();
    for (int i = 0; i < infoArray.size(); i++) {
        QJsonObject infoObj = infoArray.at(i).toObject();
        QString thumbPath = infoObj.value("thumbPath").toString();
        list.append(thumbPath);
    }
    file->close();
    file->deleteLater();
    file = nullptr;

    return;
}

bool FileParSer::isNeedUpdate(QString sourceFile, QString targetData, QString key)
{
    if (targetData.isEmpty()) {
        qCritical() << "The update content is empty!";
        return false;
    }
    // 读取源文件
    QFile *file = new QFile(sourceFile);
    bool openFlag;
    if (!file->exists()) {
        qInfo() << "FileNum file does not exists!";
        QDir downDir(sourceFile.mid(0, sourceFile.lastIndexOf("/")));
        if (!downDir.exists()) {
            // 文件夹不存在，创建文件夹
            downDir.mkpath(sourceFile.mid(0, sourceFile.lastIndexOf("/")));
        }
        // 本地没有fileNum文件，直接创建
        QString data = addAllItemInfo(targetData);
        openFlag = writeNumInfo(file, data);
        if (!openFlag) {
            return false;
        }
        return true;
    }

    openFlag = file->open(QIODevice::ReadOnly | QIODevice::Text);
    if (!openFlag) {
        // 打开文件失败
        qCritical() << "Fail to open file: FileNum";
        file->deleteLater();
        return false;
    }
    QString sourceData = file->readAll();
    file->close();

    // 目标文件时间戳
    QString targetTimeStamp = "";
    // 源文件时间戳
    QString sourceTimeStamp = "";
    bool isNeedUpdate = true;

    targetTimeStamp = getTimeStamp(targetData);
    sourceTimeStamp = getTimeStamp(sourceData);
    if (sourceTimeStamp == targetTimeStamp) {
        if (key.isEmpty()) {
            qInfo() << "FileNum does not need to be updated.";
            isNeedUpdate = false;
            file->deleteLater();
        } else {
            if (getIsUpdate(sourceData, key)) {
                qInfo() << key << " sync last required update.";
                isNeedUpdate = true;
                QString data = updateItem(sourceData, key);
                openFlag = writeNumInfo(file, data);
                if (!openFlag) {
                    return false;
                }
            } else {
                qInfo() << key << " does not need to be updated.";
                isNeedUpdate = false;
                file->deleteLater();
            }
        }
    } else {
        // 时间戳更新，更新文件内容
        qInfo() << "File information needs to be updated.";
        isNeedUpdate = true;
        QString data = updateItemInfo(sourceData, targetData, key);
        openFlag = writeNumInfo(file, data);
        if (!openFlag) {
            return false;
        }
    }

    return isNeedUpdate;
}

bool FileParSer::setKeyItemInfo(QString path, QString key)
{
    QFile *file = new QFile(path);
    bool openFlag;
    if (!file->exists()) {
        qInfo() << "FileNum file does not exists!";
        file->deleteLater();
        // 本地没有fileNum文件
        return false;
    }

    openFlag = file->open(QIODevice::ReadOnly | QIODevice::Text);
    if (!openFlag) {
        // 打开文件失败
        qCritical() << "Fail to open file: FileNum";
        file->deleteLater();
        return false;
    }
    QString sourceData = file->readAll();
    file->close();

    QString data = updateItem(sourceData, key);
    openFlag = writeNumInfo(file, data);
    if (!openFlag) {
        qInfo() << "setKeyItemInfo ERROR!";
        return false;
    }
    return true;
}

QString FileParSer::getTimeStamp(QString data, QString key)
{
    QStringList strList = data.split(LINE_SEP);
    if (key.isEmpty()) {
        // 获取fileNum版本号
        return strList.value(0);
    }
    for (int i = 1; i < strList.count(); ++i) {
        QString info = strList.value(i);
        QStringList infoList = info.split(COL_SEP);
        if (key == infoList.value(KEY_INDEX::KEYNAME)) {
            return infoList.value(KEY_INDEX::VERSION);
        }
    }
    return QString("");
}

bool FileParSer::writeNumInfo(QFile *file, QString data)
{
    if (data.isEmpty()) {
        qCritical() << "NumInfoData isEmpty!";
        return false;
    }
    bool openFlag = file->open(QIODevice::WriteOnly | QIODevice::Text);
    if (!openFlag) {
        // 打开文件失败
        qCritical() << "Fail to open file: fileNum";
        file->deleteLater();
        return openFlag;
    }
    QByteArray dataArr = data.toLatin1();
    char *buffer = dataArr.data();
    file->write(buffer);
    file->flush();
    file->close();
    file->deleteLater();

    return openFlag;
}

QString FileParSer::addAllItemInfo(QString data)
{
    QStringList strList = data.split(LINE_SEP);
    data.clear();
    data.append(strList.value(0) + LINE_SEP);
    for (int i = 1; i < strList.count(); ++i) {
        QString info = strList.value(i);
        info.append(COL_SEP + IS_UPDATED_TREE);
        if (i == strList.count() - 1) {
            data.append(info);
        } else {
            data.append(info + LINE_SEP);
        }
    }
    return data;
}

QString FileParSer::updateItemInfo(QString sourceData, QString targetData, QString key)
{
    QMap<QString, QString> targetMap;
    QMap<QString, QString> sourceMap;
    targetMap.clear();
    sourceMap.clear();
    setInfoMap(targetData, targetMap);
    setInfoMap(sourceData, sourceMap);

    QString data = "";
    data.append(targetData.split(LINE_SEP).value(0) + LINE_SEP);

    for (int i = 0; i < FILENUM_KEY_LIST.count(); i++) {
        QString keyName = FILENUM_KEY_LIST.value(i);
        QString itemData = sourceMap.value(keyName);
        QStringList infoList = itemData.split(COL_SEP);
        // 目标文件时间戳
        QString targetTimeStamp = getTimeStamp(targetData, keyName);
        // 源文件时间戳
        QString sourceTimeStamp = getTimeStamp(sourceData, keyName);

        if (sourceTimeStamp != targetTimeStamp) {
            itemData = targetMap.value(keyName);
            if (key == keyName) {
                itemData.append(COL_SEP + IS_UPDATED_FALSE);
            } else {
                itemData.append(COL_SEP + IS_UPDATED_TREE);
            }
        }
        if (i == FILENUM_KEY_LIST.count() - 1) {
            data.append(itemData);
        } else {
            data.append(itemData + LINE_SEP);
        }
    }

    return data;
}

QString FileParSer::updateItem(QString data, QString key)
{
    QStringList strList = data.split(LINE_SEP);
    QString resultData = "";
    resultData.append(strList.value(0) + LINE_SEP);
    for (int i = 1; i < strList.count(); ++i) {
        QString itemData = strList.value(i);
        QStringList infoList = itemData.split(COL_SEP);
        if (key == infoList.value(KEY_INDEX::KEYNAME)) {
            itemData = itemData.mid(0, itemData.lastIndexOf(COL_SEP) + 1);
            itemData.append(IS_UPDATED_FALSE);
        }
        if (i == strList.count() - 1) {
            resultData.append(itemData);
        } else {
            resultData.append(itemData + LINE_SEP);
        }
    }
    return resultData;
}

bool FileParSer::getIsUpdate(QString data, QString key)
{
    QStringList strList = data.split(LINE_SEP);
    if (key.isEmpty()) {
        // 获取fileNum版本号
        qCritical() << "IsUpdate key isEmpty!";
        return true;
    }
    for (int i = 1; i < strList.count(); ++i) {
        QString info = strList.value(i);
        QStringList infoList = info.split(COL_SEP);
        if (key == infoList.value(KEY_INDEX::KEYNAME) && infoList.value(KEY_INDEX::ISUPDATE) == IS_UPDATED_TREE) {
            return true;
        }
    }

    return false;
}


void FileParSer::setInfoMap(QString data, QMap<QString, QString> &map)
{
    QStringList strList = data.split(LINE_SEP);
    for (int i = 1; i < strList.count(); ++i) {
        QString info = strList.value(i);
        QStringList infoList = info.split(COL_SEP);
        map.insert(infoList.value(KEY_INDEX::KEYNAME), info);
    }
    return;
}