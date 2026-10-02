#ifndef FILEPARSER_H
#define FILEPARSER_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QUrl>

#include "fileinfo.h"

const QStringList FILENUM_KEY_LIST = {"picture",       "video",       "music",       "document",
                                      "wechatPicture", "wechatVideo", "wechatMusic", "wechatDocument",
                                      "qqPicture",     "qqVideo",     "qqMusic",     "qqDocument"};
class FileParSer : public QObject
{
    Q_OBJECT
public:
    FileParSer(QUrl url, QObject *parent = nullptr);
    ~FileParSer();

    enum KeyType {
        All = 0,    // 所有
        WeChat = 4, // 仅微信
        QQ = 8,     // 仅QQ
    };

    enum ItemType {
        PictureNum = 0, // 图片数量
        VideoNum,       // 视频数量
        MusicNum,       // 音乐数量
        DocNum,         // 文档数量
        WeChatNum,      // 微信文件数量
        QQNum,          // QQ文件数量
    };

    // 读取配置，获取分类列表项数
    void getItemCountList(QString path, FileParSer::KeyType type, QMap<int, int> &map);

    // 获取文件信息
    void getFileInfoList(QString path, QList<FileInfo> &list, QString flag = "");

    // 获取缩略图列表
    void getThumbList(QString path, QStringList &list);

    // 判断当前文件是否需要更新
    bool isNeedUpdate(QString sourceFile, QString targetData, QString key = "");

    bool setKeyItemInfo(QString path, QString key);

private:
    enum KEY_INDEX {
        KEYNAME = 0, // 名称
        NUM,         // 数量
        VERSION,     // 版本
        ISUPDATE     // 是否需要更新
    };
    QString getTimeStamp(QString data, QString key = "");
    bool writeNumInfo(QFile *file, QString data);
    QString addAllItemInfo(QString data);
    QString updateItemInfo(QString sourceData, QString targetData, QString key = "");
    QString updateItem(QString data, QString key);
    bool getIsUpdate(QString data, QString key);
    void setInfoMap(QString data, QMap<QString, QString> &map);

private:
    QUrl m_url; // 服务器地址
Q_SIGNALS:

public Q_SLOTS:
};

#endif // FILEPARSER_H