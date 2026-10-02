#include "fileprocess.h"

QString FileProcess::g_date;
QString FileProcess::g_themeCNName;
QString FileProcess::g_themeENName;
QString FileProcess::g_builderName;
QString FileProcess::g_builderMailName;
QString FileProcess::g_createThemeType;
QString FileProcess::g_debPath;
bool FileProcess::g_isSetup = false;
bool FileProcess::g_isBatchImport = false;

FileProcess::FileProcess()
{
    //创建缓存目录
    m_builder = QDir::homePath()+"/.cache/theme-build/";

    if (m_builder.exists()){
        qDebug()<<"thmem-build cache Dir exists";
    } else {
        m_builder.mkdir(m_builder.absolutePath());
        qDebug()<<"thmem-build cache Dir create";
    }
    return;
}

QMap<QString, QVariant> FileProcess::FileSearch(const QString &m_date){
    QString m_getInfoFIlePath = QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/debian/control";
    QMap<QString, QVariant> emptyMap;
    // 处理文件不存在的情况
    if (m_getInfoFIlePath.isEmpty()){
        qDebug() << m_getInfoFIlePath << "文件不存在！";
        return emptyMap;
    } else {
        QFile m_getInfoFIle(m_getInfoFIlePath);
        if (!m_getInfoFIle.open(QIODevice::ReadOnly | QIODevice::Text)){
            // 处理文件打开失败的情况
            qDebug() << m_getInfoFIlePath << "文件无法打开！";
            return emptyMap;
        } else {
            QTextStream in(&m_getInfoFIle);
            int currentLine = 1;
            QString line;
            QMap<QString, QVariant> m_packageInfo;

            while (!in.atEnd()){
                line = in.readLine();
                if (currentLine == 1){
                    m_packageInfo.insert("key1",line.mid(8));
                } else if(currentLine == 4){
                    m_packageInfo.insert("key2",line.mid(12, line.indexOf('<')-12).trimmed());
                    m_packageInfo.insert("key3",line.mid(line.indexOf('<') + 1, line.indexOf('>') - line.indexOf('<') - 1).trimmed());
                } else if(currentLine == 12){
                    m_packageInfo.insert("key4",line.mid(27));
                }
                currentLine++;
            }
            m_getInfoFIle.close();
            return m_packageInfo;
        }
        return emptyMap;
    }
}

void FileProcess::FileCreate(const QString &m_themeType){

    QDateTime currentDateTime = QDateTime::currentDateTime();
    g_date = currentDateTime.toString("yyyyMMddhhmmss");
//    qDebug()<< g_date;

    //创建新的记录文件目录
    QString m_themePath = QDir::homePath() + "/.cache/theme-build/" + g_date;
    QDir m_builderTime = m_themePath;
    QString m_builderPathSrc = m_themePath + "/src";
    QDir m_builderSrc = m_builderPathSrc;
    QDir m_builderDeb = m_builderPathSrc + "/debian";
    QDir m_themeTypePath = m_builderPathSrc + "/" + m_themeType;
    QDir appPath = m_builderPathSrc+ "/application";

    m_builderTime.mkdir(m_builderTime.absolutePath());
    m_builderSrc.mkdir(m_builderSrc.absolutePath());
    m_builderDeb.mkdir(m_builderDeb.absolutePath());
    m_themeTypePath.mkdir(m_themeTypePath.absolutePath());
    appPath.mkdir(appPath.absolutePath());

    QDir iconPath = m_builderPathSrc+ "/iconTheme";
    QDir cursorPath = m_builderPathSrc+ "/cursorTheme";
    QDir appiconPath = m_builderPathSrc + "/iconTheme" + "/appicon";
    QDir systemiconPath = m_builderPathSrc + "/iconTheme" + "/systemicon";
    QDir placeiconPath = m_builderPathSrc + "/iconTheme" + "/placeicon";
    QDir deviceiconPath = m_builderPathSrc + "/iconTheme" + "/deviceicon";
    QDir cursorIconPath = m_builderPathSrc + "/cursorTheme" + "/cursor";
    QDir timecursorPath = m_builderPathSrc + "/cursorTheme" + "/timecursor";
    QDir plymouthPath = m_builderPathSrc+ "/plymouthTheme";
    QDir grubPath = m_builderPathSrc+ "/grubTheme";

    if(m_themeType == "globalTheme"){
        iconPath.mkdir(iconPath.absolutePath());
        cursorPath.mkdir(cursorPath.absolutePath());
        appiconPath.mkdir(appiconPath.absolutePath());
        systemiconPath.mkdir(systemiconPath.absolutePath());
        placeiconPath.mkdir(placeiconPath.absolutePath());
        deviceiconPath.mkdir(deviceiconPath.absolutePath());
        cursorIconPath.mkdir(cursorIconPath.absolutePath());
        timecursorPath.mkdir(timecursorPath.absolutePath());
        plymouthPath.mkdir(plymouthPath.absolutePath());
        grubPath.mkdir(grubPath.absolutePath());

    } else if (m_themeType == "iconTheme"){
        iconPath.mkdir(iconPath.absolutePath());
        appiconPath.mkdir(appiconPath.absolutePath());
        systemiconPath.mkdir(systemiconPath.absolutePath());
        placeiconPath.mkdir(placeiconPath.absolutePath());
        deviceiconPath.mkdir(deviceiconPath.absolutePath());

    } else if (m_themeType == "cursorTheme"){
        cursorPath.mkdir(cursorPath.absolutePath());
        cursorIconPath.mkdir(cursorIconPath.absolutePath());
        timecursorPath.mkdir(timecursorPath.absolutePath());

    } else if (m_themeType == "plymouthTheme"){
        plymouthPath.mkdir(plymouthPath.absolutePath());

    } else if (m_themeType == "grubTheme"){
        grubPath.mkdir(grubPath.absolutePath());

    }
    FileProcess::g_createThemeType = m_themeType;
}

void FileProcess::FileClean(const QString &m_date){
    QDir m_datePath = QDir::homePath() + "/.cache/theme-build/" + m_date;
    m_datePath.removeRecursively();
}
