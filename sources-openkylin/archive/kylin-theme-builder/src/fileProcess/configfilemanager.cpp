/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#include "configfilemanager.h"
#include "historyinfoload.h"
#include <QProcess>
#include <QDirIterator>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QDebug>
#include <QtMath>
#include <QRegularExpression>
#include <QBuffer>
#include <QFileInfoList>

ConfigFileManager::ConfigFileManager(QObject *parent) : QObject(parent)
{

}

/**
 * @brief 创建 JSON 文件
 *
 * 在指定的缓存目录中创建 JSON 文件。
 *
 * @param cachedir 缓存目录对象
 *
 * @return 如果成功创建 JSON 文件，则返回 true；否则返回 false。
 */
bool ConfigFileManager::createJson(QDir cachedir)
{
    QString m_themePath = cachedir.absolutePath();
    QString filePath = m_themePath + "/json.txt";

    QDir dir;
    if (!dir.exists(m_themePath)) {
        dir.mkpath(m_themePath);
    }

    if (copyFileContent(":/resource/config/json.txt", filePath)) {
        // 成功将文件内容写入目标文件中
        return true;
    } else {
        // 写入文件内容失败
        return false;
    }
}

/**
 * @brief 创建配置文件
 *
 * 在给定的缓存目录中创建配置文件。
 *
 * @param cachedir 缓存目录对象
 *
 * @return 创建成功返回 true，否则返回 false
 */
bool ConfigFileManager::createConf(QDir cachedir)
{
    QString m_themePath = cachedir.absolutePath();
    QString filePath = m_themePath + "/theme.conf";

    QDir dir;
    if (!dir.exists(m_themePath)) {
        dir.mkpath(m_themePath);
    }

    if (copyFileContent(":/resource/config/theme.conf", filePath)) {
        // 成功将文件内容写入目标文件中
        return true;
    } else {
        // 写入文件内容失败
        return false;
    }
}

bool ConfigFileManager::createXml(QDir cachedir)
{
    QString m_themePath = cachedir.absolutePath();
    QString filePath = m_themePath + "/kylin-theme-builder.xml";

    QDir dir;
    if (!dir.exists(m_themePath)) {
        dir.mkpath(m_themePath);
    }

    if (copyFileContent(":/resource/config/kylin-theme-builder.txt", filePath)) {
        return true;
    } else {
        return false;
    }
}

bool ConfigFileManager::createFocalXml(QDir cachedir)
{
    QString m_themePath = cachedir.absolutePath();
    QString filePath = m_themePath + "/kylin-theme-builder-focal.xml";

    QDir dir;
    if (!dir.exists(m_themePath)) {
        dir.mkpath(m_themePath);
    }

    if (copyFileContent(":/resource/config/kylin-theme-builder.txt", filePath)) {
        return true;
    } else {
        return false;
    }
}

bool ConfigFileManager::createDesktop(QDir cachedir)
{
    QString m_desktopPath = cachedir.absolutePath();
    QString filePath = m_desktopPath + "/theme.desktop";

    QDir dir;
    if (!dir.exists(m_desktopPath)) {
        dir.mkpath(m_desktopPath);
    }

    if (copyFileContent(":/resource/theme.desktop", filePath)) {
        return true;
    } else {
        return false;
    }
}

bool ConfigFileManager::createSetupConf(QDir cachedir)
{
    QString m_themePath = cachedir.absolutePath();
    QString desktopPath = m_themePath + "/kylin-theme-builder-change.desktop";
    QString shellPath = m_themePath + "/kylin-theme-builder-change";
    QString configPath = m_themePath + "/theme-builder-change.conf";
    QString schemasPath = m_themePath + "/40_kylin-theme-builder.gschema.override";

    QDir dir;
    if (!dir.exists(m_themePath)) {
        dir.mkpath(m_themePath);
    }

    if (copyFileContent(":/resource/config/kylin-theme-builder-change.desktop", desktopPath)&&
            copyFileContent(":/resource/config/kylin-theme-builder-change", shellPath)&&
            copyFileContent(":/resource/config/theme-builder-change.conf", configPath)&&
            copyFileContent(":/resource/config/40_kylin-theme-builder.gschema.override", schemasPath)) {
        return true;
    } else {
        return false;
    }
}

bool ConfigFileManager::createSavePathConfig()
{
    QString dirPath = QDir::homePath() + "/.cache/theme-build";
    QString filePath = dirPath + "/savepath.conf";

    QDir dir;
    if (!dir.exists(dirPath)) {
        dir.mkpath(dirPath);
    }

    QFile file(filePath);

    if (!file.exists()) {
        if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << QDir::homePath() + "/桌面";
            file.close();
            FileProcess::g_debPath = QDir::homePath() + "/桌面";
            return true;
        } else {
            return false;
        }
    } else {
        if (file.open(QIODevice::ReadWrite| QIODevice::Text)) {
            QTextStream in(&file);
            FileProcess::g_debPath = in.readAll();
            file.close();
        }
        return true;
    }
}

/**
 * @brief 修改radius的json配置文件
 *
 * 根据给定的radius值，修改指定的json配置文件中的"Radius"字段的值。
 *
 * @param radius Radius值
 * @param jsonFilePath 配置文件路径
 *
 * @return 修改成功返回true，否则返回false
 */
bool ConfigFileManager::modifyRadiusJson(int radius,QString jsonFilePath)
{
    // 读取文件内容
    QFile file(jsonFilePath);
    if (!file.open(QIODevice::ReadWrite| QIODevice::Text)) {
        qDebug() << "Failed to open file for reading";
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 使用正则表达式进行替换
    QRegularExpression regexMax(R"("Max_Radius"\s*:\s*{"value"\s*:\s*"\d+"\s*,\s*"type"\s*:\s*"int"\s*})");
    content.replace(regexMax, "\"Max_Radius\"    :  {\"value\":  \"" + QString::number(radius) + "\", \"type\": \"int\"}");

    QRegularExpression regexNormal(R"("Normal_Radius"\s*:\s*{"value"\s*:\s*"\d+"\s*,\s*"type"\s*:\s*"int"\s*})");
    content.replace(regexNormal, "\"Normal_Radius\" :  {\"value\":  \"" + QString::number(radius) + "\", \"type\": \"int\"}");

    QRegularExpression regexMin(R"("Min_Radius"\s*:\s*{"value"\s*:\s*"\d+"\s*,\s*"type"\s*:\s*"int"\s*})");
    content.replace(regexMin, "\"Min_Radius\"    :  {\"value\":  \"" + QString::number(radius) + "\", \"type\": \"int\"}");

//    qDebug()<<QString::number(radius);

    // 写回文件，限制输出行数为最多 927 行
    if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for writing";
        return false;
    }

    QTextStream out(&file);
    QStringList lines = content.split("\n");
    int linesToWrite = qMin(lines.size(), 927); // 取最小值，确保不超过 927 行

    for (int i = 0; i < linesToWrite; ++i) {
        out << lines[i] << "\n";
    }

    file.close();

    qDebug() << "Config file updated successfully";
    emit updateInfo();

    return true;
}
/**
 * @brief 修改配置文件中的强调色配置
 *
 * 将给定的强调色配置写入指定的配置文件中。
 *
 * @param accentcolor 强调色
 * @param confFilePath 配置文件路径
 *
 * @return 修改成功返回 true，否则返回 false
 */
bool ConfigFileManager::modifyAccentColorConf(QColor accentcolor,QString confFilePath)
{

    QString colorString = accentcolor.name().toUpper();

    while (colorString.length() < 6) {
        colorString.prepend('0');
    }

    // 读取文件内容
    QFile file(confFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << confFilePath;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 修改内容
    int startIndex = content.indexOf("accent=") + QString("accent=").size();
    content.remove(startIndex, 7);
    content.insert(startIndex, colorString);
    // 写入文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
        file.close();
        emit updateInfo();
        return true;
    } else {
        qDebug() << "无法写入文件：" << confFilePath;
        return false;
    }
}

/**
 * @brief 修改配置文件中的强调色配置
 *
 * 将给定的强调色配置写入指定的配置文件中。
 * 注意，modifyQtAccentColorConf用于修改qt主题框架的参数，modifyAccentColorConf用于修改gtk主题的参数
 * @param accentcolor 强调色
 * @param confFilePath 配置文件路径
 *
 * @return 修改成功返回 true，否则返回 false
 */
bool ConfigFileManager::modifyQtAccentColorConf(QString accentcolor, QString confFilePath)
{
    QString colorString = accentcolor;

    while (colorString.length() < 6) {
        colorString.prepend('0');
    }

    // 读取文件内容
    QFile file(confFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << confFilePath;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 修改内容
    int startIndex = content.indexOf("accentRole=") + QString("accentRole=").size();
    int endIndex = content.indexOf("\n", startIndex);
    content.remove(startIndex, endIndex - startIndex);
    content.insert(startIndex, colorString);

    // 写入文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
        file.close();
        emit updateInfo();
        return true;
    } else {
        qDebug() << "无法写入文件：" << confFilePath;
        return false;
    }
}

/**
 * @brief 修改壁纸配置文件
 *
 * 从指定的配置文件路径读取文件内容，并修改其中的壁纸路径，然后将修改后的内容写回到文件中。
 *
 * @param wallpaperpath 壁纸路径
 * @param confFilePath 配置文件路径
 *
 * @return 修改成功返回 true，否则返回 false
 */
bool ConfigFileManager::modifyWallPaperConf(QString wallpaperpath,QString confFilePath)
{

    // 读取文件内容
    QFile file(confFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << confFilePath;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QFile file1(wallpaperpath);
    QFileInfo info(file1);
    QString suffix = info.suffix();
    // 修改内容
    int startIndex = content.indexOf("wallPaperPath=/usr/share/backgrounds/1-openkylin.");
    if (startIndex != -1) {
        QString substr = "wallPaperPath=/usr/share/backgrounds/1-openkylin."+suffix;
        content.replace(startIndex, content.indexOf('\n', startIndex) - startIndex, substr);
    }

    // 写入文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
        file.close();
        emit updateInfo();
        return true;
    } else {
        qDebug() << "无法写入文件：" << confFilePath;
        return false;
    }
}


/**
 * @brief 修改部件样式配置
 *
 * 根据给定的部件样式和配置文件路径，修改配置文件中的部件样式配置。
 *
 * @param WidgetStyle 部件样式
 * @param confFilePath 配置文件路径
 *
 * @return 修改成功返回 true，否则返回 false
 */
bool ConfigFileManager::modifyWidgetStyleConf(QString WidgetStyle, QString confFilePath)
{
    QString colorString = WidgetStyle;

    // 读取文件内容
    QFile file(confFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << confFilePath;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 修改内容
    int startIndex = content.indexOf("defaultLightDarkMode=") + QString("defaultLightDarkMode=").size();
    int endIndex = content.indexOf("\n", startIndex);
    content.remove(startIndex, endIndex - startIndex);
    content.insert(startIndex, colorString);

    // 写入文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
        file.close();
        emit updateInfo();
        return true;
    } else {
        qDebug() << "无法写入文件：" << confFilePath;
        return false;
    }
}

/**
 * @brief 修改透明度配置
 *
 * 读取指定的配置文件内容，并修改其中的透明度设置，最后将修改后的内容写回到文件中。
 *
 * @param transparency 透明度值
 * @param confFilePath 配置文件路径
 *
 * @return 修改成功返回 true，否则返回 false
 */
bool ConfigFileManager::modifyTransparencyConf(int transparency,QString confFilePath)
{
    // 读取文件内容
    QFile file(confFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << confFilePath;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 修改内容
    QString oldValue = "transparencyBlur=";
    QString newValue = QString("transparencyBlur=%1").arg(transparency);
    content.replace(QRegExp(oldValue + "\\d+"), newValue);

    // 写入文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
        file.close();
        emit updateInfo();
        return true;
    } else {
        qDebug() << "无法写入文件：" << confFilePath;
        return false;
    }
}

/**
 * @brief 修改窗口半径配置
 *
 * 将指定的文件中的窗口半径配置进行修改，并保存到文件中。
 *
 * @param windowradius 窗口半径值
 * @param confFilePath 配置文件路径
 *
 * @return 修改成功返回 true，否则返回 false
 */
bool ConfigFileManager::modifywindowRadiusConf(int windowradius,QString confFilePath)
{
    QFile file(confFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file: " << confFilePath;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QString oldValue = "windowRadius=";
    QString newValue = QString("windowRadius=%1").arg(windowradius);
    content.replace(QRegExp(oldValue + "\\d+"), newValue);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
        file.close();
        emit updateInfo();
        return true;
    } else {
        qDebug() << "Failed to write file: " << confFilePath;
        return false;
    }
}

bool ConfigFileManager::modifyFocalXml(QString focalXmlPath)
{
    QString themeName = FileProcess::g_themeENName;
    // 读取文件内容
    QFile file(focalXmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << focalXmlPath;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 替换文件名
    QString newFilename = "/usr/share/backgrounds/kylin-theme-builder-focal-" + themeName + ".jpg";
    content.replace("/usr/share/backgrounds/2-warty-final-ubuntukylin.jpg", newFilename);

    // 写入修改后的内容到文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法写入文件：" << focalXmlPath;
        return false;
    }

    QTextStream out(&file);
    out << content;
    file.close();

    return true;
}

bool ConfigFileManager::modifyDesktop(QString desktop)
{
    QString themeName = FileProcess::g_themeENName;
    // 读取文件内容
    QFile file(desktop);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件：" << desktop;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 替换文件名
    QString newFilename = "Name=" + themeName;
    content.replace("Name=", newFilename);

    // 写入修改后的内容到文件
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法写入文件：" << desktop;
        return false;
    }

    QTextStream out(&file);
    out << content;
    file.close();

    return true;
}

bool ConfigFileManager::modifySavePathConf(QString savePath)
{
    QString dirPath = QDir::homePath() + "/.cache/theme-build";
    QString filePath = dirPath + "/savepath.conf";

    QFile file(filePath);

    // Open the file in WriteOnly mode to clear the content
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream stream(&file);
        stream << savePath;
        file.close();
        return true;
    } else {
        // Handle the case when the file cannot be opened for writing
        return false;
    }
}

/**
 * @brief 复制文件内容到另一个文件
 *
 * 将源文件的内容复制到目标文件中。
 *
 * @param sourceFilePath 源文件路径
 * @param destinationFilePath 目标文件路径
 *
 * @return 复制成功返回 true，否则返回 false
 */
bool ConfigFileManager::copyFileContent(const QString &sourceFilePath, const QString &destinationFilePath)
{
    QFile sourceFile(sourceFilePath);
    if (!sourceFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open source file:" << sourceFilePath;
        return false;
    }

    QFile destinationFile(destinationFilePath);
    if (!destinationFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open destination file:" << destinationFilePath;
        sourceFile.close();
        return false;
    }
    QTextStream in(&sourceFile);
    QTextStream out(&destinationFile);

    while (!in.atEnd()) {
        QString line = in.readLine();
        out << line << "\n";
    }

    sourceFile.close();
    destinationFile.close();

    return true;
}

/**
 * @brief 将文件复制到缓存目录
 *
 * 将指定的文件复制到缓存目录中。如果文件已经存在于缓存目录中，则不会进行复制。
 *
 * @param map 存储键值对的映射
 * @param cachedir 缓存目录对象
 *
 * @return 复制是否成功的布尔值
 */
bool ConfigFileManager::copyIcontoCacheDir(QMap<QString, QString> *map, QDir cachedir)
{
    bool success = true;
    // Delete files not in the map
    QStringList fileList = cachedir.entryList(QDir::Files);
    for (const QString &file : fileList) {
        if (!map->values().contains(cachedir.filePath(file))) {
            QFile::remove(cachedir.filePath(file));
        }
    }

    // Copy files from map to cache directory
    for (auto it = map->begin(); it != map->end(); ++it)
    {
        QString key = it.key();
        QString value = it.value();

        QFile inputFile(value);
        QFileInfo fileInfo(inputFile.fileName());
        QString destinationPath = cachedir.filePath(key + "." + fileInfo.completeSuffix());
        if (!inputFile.exists()){
            success = false;
            continue;
        }

        if (!inputFile.copy(destinationPath)){
            success = false;
        }
    }

    return success;
}

/**
 * @brief 将文件复制到缓存目录
 *
 * 将指定的文件复制到缓存目录中，根据不同的类型设置不同的目标文件名。
 *
 * @param destinationFilePath 目标文件路径
 * @param cachedir 缓存目录
 * @param type 类型
 */
void ConfigFileManager::copyPictoCacheDir(const QString &destinationFilePath, QDir cachedir, QString type)
{
    QString sourceFilePath = destinationFilePath;
    QString sourceFileExt = QFileInfo(sourceFilePath).suffix();

    QString targetFileName = "";
    if(type == "cover"){
        targetFileName = "preview." + sourceFileExt;
    }else if (type == "grub"){
        targetFileName = "background." + sourceFileExt;
    }else if (type == "wallpaper"){
        targetFileName = "1-openkylin." + sourceFileExt;
        QFile::remove(cachedir.filePath("1-openkylin.png"));
        QFile::remove(cachedir.filePath("1-openkylin.jpg"));
    }

    QString targetFilePath = cachedir.filePath(targetFileName);
    QFile::remove(targetFilePath);

    if (!QFile::copy(sourceFilePath, targetFilePath))
    {
        qDebug() << "Failed to copy file";
        return;
    }
    emit updateInfo();
    qDebug() << "File copied successfully";
}

void ConfigFileManager::copyPicstoCacheDir(const QString &destinationFilePath, QDir cachedir)
{
    //清空文件夹
    QStringList fileList = cachedir.entryList(QDir::Files);
    foreach (const QString &fileName, fileList) {
        QString filePath = cachedir.absoluteFilePath(fileName);
        QFile::remove(filePath);
    }

    QFile sourceFile(destinationFilePath);

    if (!sourceFile.exists()) {
        qDebug() << "Source file does not exist!";
        return;
    }

    QDir destinationDirectory(cachedir);
    if (!destinationDirectory.exists()) {
        qDebug() << "Destination directory does not exist!";
        return;
    }

//    QString baseFileName = QFileInfo(sourceFile).baseName();
    QString fileExtension = QFileInfo(sourceFile).completeSuffix();
    QString destinationPath = destinationDirectory.absolutePath(); // Convert QDir to QString

    for (int i = 1; i <= 104; ++i) {
        QString newFileName = QString("%1/%2.%4").arg(destinationPath).arg(i).arg(fileExtension);
        QFile destinationFile(newFileName);

        if (destinationFile.exists()) {
            qDebug() << "File" << newFileName << "already exists. Skipping.";
            continue;
        }

        if (sourceFile.copy(newFileName)) {
//            qDebug() << "Copied" << destinationFilePath << "to" << newFileName;
        } else {
            qDebug() << "Failed to copy" << destinationFilePath << "to" << newFileName;
        }
    }
    QString thunbnail = cachedir.absolutePath()+"/40.png";
    emit updateThumbnail(thunbnail);
}

/**
 * @brief 将文件复制到缓存目录
 *
 * 使用ffmpeg命令将指定的MP4文件复制到缓存目录中，并根据文件名生成相应的缓存文件。
 *
 * @param destinationFilePath 目标文件路径
 * @param cachedir 缓存目录
 */
void ConfigFileManager::copyMp4toCacheDir(const QString &destinationFilePath, QDir cachedir)
{
    //清空文件夹
    QStringList fileList = cachedir.entryList(QDir::Files);
    foreach (const QString &fileName, fileList) {
        QString filePath = cachedir.absoluteFilePath(fileName);
        QFile::remove(filePath);
    }

    QStringList list;
    //ffmpeg -i "111.mp4" -vframes 104 "/home/.cache/theme-build/m_time/src/plymouthTheme/%d.png"
    //FIX ME :限制条件
    QString cache = cachedir.absolutePath()+"/%d.png";
    list<<"-i"<<destinationFilePath
        <<"-vf"
        <<"scale='min(100,iw)':min'(100,ih)':force_original_aspect_ratio=decrease"
        <<"-vframes"
        <<"104"
        <<cache;

    QProcess p;
    p.start("ffmpeg",list);
    if (!p.waitForStarted()) {
        qWarning()<<"start get video image failed.";
    }

    if (!p.waitForFinished(40000)) {
        qWarning()<<"wait video image too long time.";
    }
    QString err=p.readAllStandardError();
    if (err.contains("not contain any stream")) {
        qWarning()<<"get video image failed.";
    }

    QString thunbnail = cachedir.absolutePath()+"/40.png";
    emit updateThumbnail(thunbnail);
}

void ConfigFileManager::copyAllPictoCacheDir(QDir sourcedir, QDir cachedir)
{
    //清空文件夹
    QStringList fileList1 = cachedir.entryList(QDir::Files);

    foreach (const QString &fileName, fileList1) {
        QString filePath = cachedir.absoluteFilePath(fileName);
        QFile::remove(filePath);
    }
    QDir sourceDirectory(sourcedir);
    QDir destinationDirectory(cachedir);

    if (!sourceDirectory.exists()) {
        qDebug() << "Source directory does not exist!";
        return;
    }

    if (!destinationDirectory.exists()) {
        qDebug() << "Destination directory does not exist!";
        return;
    }

    QStringList filters;
    filters << "*.png";

    sourceDirectory.setNameFilters(filters);
    QFileInfoList fileList = sourceDirectory.entryInfoList();

    foreach (QFileInfo fileInfo, fileList) {
        QString baseFileName = fileInfo.baseName();
        QString fileExtension = fileInfo.completeSuffix();
        QString destinationPath = destinationDirectory.absoluteFilePath(QString("%1.%2").arg(baseFileName).arg(fileExtension));

        QFile destinationFile(destinationPath);

        if (destinationFile.exists()) {
            qDebug() << "File" << destinationPath << "already exists. Skipping.";
            continue;
        }

        if (QFile::copy(fileInfo.filePath(), destinationPath)) {
            qDebug() << "Copied" << fileInfo.filePath() << "to" << destinationPath;
        } else {
            qDebug() << "Failed to copy" << fileInfo.filePath() << "to" << destinationPath;
        }
    }
    QString thunbnail = cachedir.absolutePath()+"/40.png";
    emit updateThumbnail(thunbnail);
}

