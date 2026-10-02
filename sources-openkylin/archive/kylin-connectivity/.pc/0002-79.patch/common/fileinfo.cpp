#include "fileinfo.h"

#include <QMimeDatabase>
#include <QMap>
#include <QDebug>

QMap<QString, QString> FileInfo::mimeMapTable = FileInfo::initMimeMap();
QMap<QString, QString> FileInfo::iconNameMapTable = FileInfo::initIconNameMap();
QMap<QString, QString> FileInfo::initMimeMap()
{
    QMap<QString, QString> map;
    // for supported image types
    map.insert("bmp", "image/x-ms-bmp");
    map.insert("gif", "image/gif");
    map.insert("jpg", "image/jpeg");
    map.insert("jpeg", "image/jpeg");
    map.insert("png", "image/png");
    map.insert("pbm", "image/pbm");
    map.insert("pgm", "image/pgm");
    map.insert("ppm", "image/ppm");
    map.insert("xbm", "image/xbm");
    map.insert("xpm", "image/xpm");
    map.insert("svg", "image/svg");
    map.insert("dds", "image/dds");
    map.insert("icns", "image/icns");
    map.insert("jp2", "image/jp2");
    map.insert("mng", "image/mng");
    map.insert("tga", "image/tga");
    map.insert("tiff", "image/tiff");
    map.insert("tif", "image/tif");
    map.insert("wbmp", "image/wbmp");
    map.insert("webp", "image/webp");
    // for supported video types
    map.insert("avi", "video/x-msvideo");
    map.insert("vfw", "video/vfw");
    map.insert("divx", "video/divx");
    map.insert("mpg", "video/mpg");
    map.insert("mpe", "video/mpeg");
    map.insert("mpeg", "video/mpeg");
    map.insert("mpg", "video/mpeg");
    map.insert("mpg4", "video/mp4");
    map.insert("m1v", "video/m1v");
    map.insert("m2v", "video/m2v");
    map.insert("mpv", "video/mpv");
    map.insert("dv", "video/dv");
    map.insert("3gp", "video/3gpp");
    map.insert("3g2", "video/3gpp2");
    map.insert("mov", "video/quicktime");
    map.insert("mp4", "video/mp4");
    map.insert("m4v", "video/x-m4v");
    map.insert("dat", "video/dat");
    map.insert("vcd", "video/vcd");
    map.insert("ogm", "video/ogm");
    map.insert("ogv", "video/ogv");
    map.insert("ogx", "video/ogx");
    map.insert("asf", "video/x-ms-asf");
    map.insert("wmv", "video/x-ms-wmv");
    map.insert("iso", "video/iso");
    map.insert("vob", "video/vob");
    map.insert("mkv", "video/mkv");
    map.insert("nsv", "video/nsv");
    map.insert("ram", "video/ram");
    map.insert("flv", "video/flv");
    map.insert("rm", "video/rm");
    map.insert("swf", "video/swf");
    map.insert("ts", "video/ts");
    map.insert("rmvb", "video/rmvb");
    map.insert("dvr-ms", "video/dvr-ms");
    map.insert("m2t", "video/m2t");
    map.insert("m2ts", "video/m2ts");
    map.insert("mts", "video/mts");
    map.insert("rec", "video/rec");
    map.insert("wtv", "video/wtv");
    map.insert("f4v", "video/f4v");
    map.insert("hdmov", "video/hdmov");
    map.insert("webm", "video/webm");
    map.insert("vp8", "video/vp8");
    map.insert("bik", "video/bik");
    map.insert("smk", "video/smk");
    map.insert("wtv", "video/wtv");
    map.insert("part", "video/part");
    map.insert("m4u", "video/vnd.mpegurl");
    // for supported audio types
    map.insert("mp2", "audio/x-mpeg");
    map.insert("mp3", "audio/x-mpeg");
    map.insert("ogg", "audio/ogg");
    map.insert("oga", "audio/oga");
    map.insert("wav", "audio/x-wav");
    map.insert("wma", "audio/x-ms-wma");
    map.insert("aac", "audio/aac");
    map.insert("ac3", "audio/ac3");
    map.insert("dts", "audio/dts");
    map.insert("ra", "audio/ra");
    map.insert("ape", "audio/ape");
    map.insert("flac", "audio/flac");
    map.insert("thd", "audio/thd");
    map.insert("mka", "audio/mka");
    map.insert("m4a", "audio/mp4a-latm");
    map.insert("opus", "audio/opus");
    map.insert("m3u", "audio/x-mpegurl");
    map.insert("m4b", "audio/mp4a-latm");
    map.insert("m4p", "audio/mp4a-latm");
    map.insert("mpga", "audio/mpeg");
    map.insert("aif", "audio/x-aiff");
    // for supported document types
    map.insert("txt", "text/plain");
    map.insert("rar", "application/vnd.rar");
    map.insert("java", "text/x-java");
    map.insert("apk", "application/vnd.android.package-archive");
    map.insert("zip", "application/zip");
    map.insert("deb", "application/vnd.debian.binary-package");
    map.insert("udeb", "application/vnd.debian.binary-package");
    map.insert("dot", "application/msword-template");
    map.insert("pot", "text/x-gettext-translation-template");
    map.insert("docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    map.insert("doc", "application/msword");
    map.insert("xls", "application/vnd.ms-excel");
    map.insert("et", "application/vnd.ms-excel");
    map.insert("xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    map.insert("pps", "application/vnd.ms-powerpoint");
    map.insert("ppt", "application/vnd.ms-powerpoint");
    map.insert("dps", "application/vnd.ms-powerpoint");
    map.insert("pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation");
    map.insert("wps", "application/vnd.ms-works");
    map.insert("bin", "application/octet-stream");
    map.insert("c", "text/plain");
    map.insert("class", "application/octet-stream");
    map.insert("conf", "text/plain");
    map.insert("cpp", "text/plain");
    map.insert("c", "text/plain");
    map.insert("h", "text/plain");
    map.insert("htm", "text/html");
    map.insert("html", "ttext/html");
    map.insert("jar", "application/java-archive");
    map.insert("java", "text/plain");
    map.insert("js", "application/x-javascript");
    map.insert("exe", "application/octet-stream");
    map.insert("gtar", "application/x-gtar");
    map.insert("gz", "application/x-gzip");
    map.insert("log", "text/plain");
    map.insert("mpc", "application/vnd.mpohun.certificate");
    map.insert("msg", "application/vnd.ms-outlook");
    map.insert("prop", "text/plain");
    map.insert("rc", "text/plain");
    map.insert("rtf", "application/rtf");
    map.insert("sh", "text/plain");
    map.insert("tar", "application/x-tar");
    map.insert("tgz", "application/x-compressed");
    map.insert("rc", "text/plain");
    map.insert("xml", "text/xml");  // text/plain
    map.insert("z", "application/x-compress");
    map.insert("zip", "application/x-zip-compressed");
    map.insert("pdf", "application/pdf");
    return map;
}

QMap<QString, QString> FileInfo::initIconNameMap()
{
    QMap<QString, QString> map;
    // for supported image icon
    map.insert("image", "image-x-generic");
    // for supported video icon
    map.insert("video", "video-x-generic");
    // for supported audio icon
    map.insert("audio/x-mpeg", "audio-x-generic");
    // for supported document types
    map.insert("text", "text");
    map.insert("application/vnd.rar", "rar");
    map.insert("application/vnd.android.package-archive", "application-apk");
    map.insert("application/zip", "zip");
    map.insert("application/vnd.debian.binary-package", "application-vnd.debian.binary-package");
    map.insert("application/vnd.openxmlformats-officedocument.wordprocessingml.document",
               "application-vnd.openxmlformats-officedocument.wordprocessingml.document");
    map.insert("application/msword", "application-msword");
    map.insert("application/vnd.ms-works", "application-msword");
    map.insert("application/vnd.ms-excel", "application-vnd.ms-excel");
    map.insert("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
               "application-vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    map.insert("application/vnd.ms-powerpoint", "application-vnd.ms-powerpoint");
    map.insert("application/vnd.openxmlformats-officedocument.presentationml.presentation",
               "application-vnd.openxmlformats-officedocument.presentationml.presentation");
    map.insert("application/octet-stream", "application-octet-stream");
    map.insert("application/java-archive", "application-x-java-archive");
    map.insert("application/x-javascript", "application-x-javascript");
    map.insert("application/x-gzip", "application-gzip");
    map.insert("application/rtf", "application-rtf");
    map.insert("application/x-tar", "application-x-tar");
    map.insert("application/x-compressed", "application-x-compress");
    map.insert("application/x-compress", "application-x-compress");
    map.insert("application/x-zip-compressed", "application-x-compress");
    map.insert("application/pdf", "application-x-pdf");
    // inode/directory
    map.insert("inode/directory", "folder");
    return map;
}

FileInfo::FileInfo()
{
    m_id = 0;
    m_name = "";
    m_path = "";
    m_size = 0;
    m_type = 0;
    m_date = "";
    m_dateTime = "";
    m_thumbnailPath = "";
    m_contentType = "";
}
FileInfo::FileInfo(const FileInfo &fileInfo)
{
    m_id = fileInfo.m_id;
    m_name = fileInfo.m_name;
    m_path = fileInfo.m_path;
    m_size = fileInfo.m_size;
    m_type = fileInfo.m_type;
    m_date = fileInfo.m_date;
    m_dateTime = fileInfo.m_dateTime;
    m_thumbnailPath = fileInfo.m_thumbnailPath;
    m_contentType = fileInfo.m_contentType;
}

FileInfo &FileInfo::operator=(const FileInfo &fileInfo)
{
    if (this == &fileInfo) {
        return *this;
    }

    m_id = fileInfo.m_id;
    m_name = fileInfo.m_name;
    m_path = fileInfo.m_path;
    m_size = fileInfo.m_size;
    m_type = fileInfo.m_type;
    m_date = fileInfo.m_date;
    m_dateTime = fileInfo.m_dateTime;
    m_thumbnailPath = fileInfo.m_thumbnailPath;
    m_contentType = fileInfo.m_contentType;
    return *this;
}

QDBusArgument &operator<<(QDBusArgument &argument, const FileInfo &fileInfo)
{
    argument.beginStructure();
    argument << fileInfo.m_id;
    argument << fileInfo.m_name;
    argument << fileInfo.m_path;
    argument << fileInfo.m_size;
    argument << fileInfo.m_type;
    argument << fileInfo.m_date;
    argument << fileInfo.m_dateTime;
    argument << fileInfo.m_thumbnailPath;
    argument << fileInfo.m_contentType;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, FileInfo &fileInfo)
{
    argument.beginStructure();
    argument >> fileInfo.m_id;
    argument >> fileInfo.m_name;
    argument >> fileInfo.m_path;
    argument >> fileInfo.m_size;
    argument >> fileInfo.m_type;
    argument >> fileInfo.m_date;
    argument >> fileInfo.m_dateTime;
    argument >> fileInfo.m_thumbnailPath;
    argument >> fileInfo.m_contentType;
    argument.endStructure();
    return argument;
}

void FileInfo::registerMetaType()
{
    qRegisterMetaType<FileInfo>("FileInfo");
    qDBusRegisterMetaType<FileInfo>();
}

QString FileInfo::name() const
{
    return m_name;
}

void FileInfo::setName(const QString &name)
{
    m_name = name;
    m_type = int(judgmentType(name));
    QString suffix = name.mid(name.lastIndexOf(".") + 1, name.size());
    m_contentType = FileInfo::contentType(suffix);
}

qint64 FileInfo::size() const
{
    return m_size;
}

void FileInfo::setSize(const qint64 &size)
{
    m_size = size;
}

FileInfo::FileType FileInfo::type() const
{
    return FileInfo::FileType(m_type);
}

void FileInfo::setType(FileInfo::FileType type)
{
    m_type = int(type);
}

QString FileInfo::dateTime() const
{
    return m_dateTime;
}

void FileInfo::setDateTime(const QString &dateTime)
{
    m_dateTime = dateTime;
}

QString FileInfo::getThumbnailPath() const
{
    return m_thumbnailPath;
}

void FileInfo::setThumbnailPath(const QString &value)
{
    m_thumbnailPath = value;
}

int FileInfo::getId() const
{
    return m_id;
}

void FileInfo::setId(int value)
{
    m_id = value;
}

QString FileInfo::getPath() const
{
    return m_path;
}

void FileInfo::setPath(const QString &path)
{
    m_path = path;
}

QString FileInfo::getDate() const
{
    return m_date;
}

void FileInfo::setDate(const QString &date)
{
    m_date = date;
}
QString FileInfo::getContentType() const
{
    qDebug() << m_name << m_type << m_contentType;
    return m_contentType;
}

FileInfo::FileType FileInfo::judgmentType(QString fileName)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(fileName);
    QString fileType = fileName.mid(fileName.lastIndexOf(".") + 1, fileName.size());
    if (mime.name().startsWith("text/")) {
        // 文本文件 text/plain, text/html, text/css, text/javascript
        return FileType::Txt;
    } else if (mime.name().startsWith("image/")) {
        // 图片 image/gif, image/png, image/jpeg, image/bmp, image/webp
        return FileType::Image;
    } else if (mime.name().startsWith("audio/")) {
        // 音频 audio/wav, audio/mpeg, audio/midi, audio/webm, audio/ogg
        return FileType::Audio;
    } else if (mime.name().startsWith("video/")) {
        // 视频 video/mp4, video/x-flv, video/webm, video/ogg
        return FileType::Video;
    } else if (fileType == "doc" || fileType == "docx" || fileType == "wps") {
        // word doc docx
        return FileType::Doc;
    } else if (fileType == "pdf") {
        // pdf
        return FileType::Pdf;
    } else if (fileType == "xlsx" || fileType == "xls" || fileType == "et") {
        // excel
        return FileType::Excel;
    } else if (fileType == "pptx" || fileType == "dps") {
        // ppt
        return FileType::Ppt;
    } else if (fileType == "zip") {
        // zip
        return FileType::Zip;
    } else {
        // 其他
        return FileType::Other;
    }
}

void FileInfo::fileOpen(QString fileName)
{
    qInfo() << "process open" << fileName;
    QString filePath = QByteArray::fromPercentEncoding(fileName.toUtf8());
    FileInfo::FileType fileType = judgmentType(fileName);
    QString command = "";
    switch (fileType) {
        case FileInfo::FileType::Image:
            command = QString("kylin-photo-viewer");
            break;
        case FileInfo::FileType::Video:
            command = QString("kylin-video");
            break;
        case FileInfo::FileType::Txt:
            command = QString("pluma");
            break;
        case FileInfo::FileType::Audio:
            command = QString("kylin-music");
            break;
        case FileInfo::FileType::Doc:
            command = QString("wps");
            break;
        case FileInfo::FileType::Excel:
            command = QString("et");
            break;
        case FileInfo::FileType::Pdf:
            command = QString("wpspdf");
            break;
        case FileInfo::FileType::Ppt:
            command = QString("wpp");
            break;
        case FileInfo::FileType::Zip:
            command = QString("engrampa");
            break;
        case FileInfo::FileType::Other:
            command = QString("pluma");
            break;
        default:
            command = QString("pluma");
            break;
    }
    QProcess::startDetached(command, QStringList() << filePath);
}

QString FileInfo::contentType(QString flag)
{
    if (!mimeMapTable.contains(flag)) {
        return QString(FILEINFO_CONTENT_TYPE_UNKNOWN);
    }
    return mimeMapTable.find(flag).value();
}

QString FileInfo::iconName(QString flag)
{
    if (!iconNameMapTable.contains(flag)) {
        return QString(FILEINFO_CONTENT_TYPE_UNKNOWN);
    }
    return iconNameMapTable.find(flag).value();
}
