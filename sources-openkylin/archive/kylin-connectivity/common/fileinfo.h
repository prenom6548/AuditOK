#ifndef FILEINFO_H
#define FILEINFO_H

#include <QString>
#include <QVariant>
#include <QProcess>
#include <QtDBus>
#include <QSet>

#define FILEINFO_CONTENT_TYPE_IMAGE "image"
#define FILEINFO_CONTENT_TYPE_VIDEO "video"
#define FILEINFO_CONTENT_TYPE_AUDIO "audio"
#define FILEINFO_CONTENT_TYPE_TEXT "text"
#define FILEINFO_CONTENT_TYPE_UNKNOWN "unknown"

class FileInfo
{
public:
    enum FileType {
        Dir = 0,  // 文件夹
        Txt,      // 文本文件
        Audio,    // 音频
        Video,    // 视频
        Image,    // 图片
        Doc,      // word
        Pdf,      // pdf
        Excel,    // excel
        Ppt,      // ppt
        Zip,      // zip
        Other,    // 其他
    };

public:
    FileInfo();
    FileInfo(const FileInfo &fileInfo);

    FileInfo &operator=(const FileInfo &fileInfo);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const FileInfo &fileInfo);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, FileInfo &fileInfo);

    static void registerMetaType();

    QString name() const;
    void setName(const QString &name);

    qint64 size() const;
    void setSize(const qint64 &size);

    FileInfo::FileType type() const;
    void setType(FileInfo::FileType type);

    QString dateTime() const;
    void setDateTime(const QString &dateTime);

    QString getThumbnailPath() const;
    void setThumbnailPath(const QString &value);

    int getId() const;
    void setId(int value);

    QString getPath() const;
    void setPath(const QString &path);

    QString getDate() const;
    void setDate(const QString &date);

    QString getContentType() const;

    static FileInfo::FileType judgmentType(QString fileName);
    static void fileOpen(QString fileName);
    static QMap<QString, QString> initMimeMap();
    static QMap<QString, QString> initIconNameMap();
    static QSet<QString> initimageSet();
    static QSet<QString> inittextSet();
    static QSet<QString> initaudioSet();
    static QSet<QString> initvideoSet();
    static QString contentType(QString flag);
    static QString iconName(QString flag);

    operator QVariant() const
    {
        return QVariant::fromValue(*this);
    }

private:
    int m_id = 0;                  // 文件id
    QString m_name = "";           // 文件名
    QString m_path = "";           // 文件路径
    qint64 m_size = 0;             // 文件大小
    int m_type = 0;                // 文件类型
    QString m_date = "";           // 时间
    QString m_dateTime = "";       // 详细时间
    QString m_thumbnailPath = "";  // 缩略图路径
    QString m_contentType = "";    // 文件类型描述
    static QMap<QString, QString> mimeMapTable;
    static QMap<QString, QString> iconNameMapTable;
    static QSet<QString> imageTypeSet;
    static QSet<QString> textTypeSet;
    static QSet<QString> audioTypeSet;
    static QSet<QString> videoTypeSet;
};
Q_DECLARE_METATYPE(FileInfo)
#endif  // FILEINFO_H
