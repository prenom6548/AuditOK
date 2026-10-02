/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "bridge.h"
#include <QMessageBox>
#include <QFile>
#include <QProcess>
#include "libkydatacollect.h"

Bridge* Bridge::instance()
{
    static Bridge s_obj;
    return &s_obj;
}

Bridge::Bridge()
{
    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        settings = new QGSettings("org.ukui.style");
       connect(settings,&QGSettings::changed,this,&Bridge::slot_SettingsChange);
    }
}

void Bridge::getMdPathOther(QString attrStr)
{
    qDebug() << Q_FUNC_INFO;
     // 将appName、path、title传递过过去
    QStringList attrList = attrStr.split(",");
    QString appName,appPath,appTitle;
    QStringList infoList;
    for(int i = 0;i<attrList.size();i++){
        appName= attrList[i].split("|")[0];
        appPath= this->js_getIndexMdFilePathOther(appName);
        appTitle= this->js_getIndexMdFileTitle(appPath);
        infoList.append(attrList[i]+"|"+appPath+"|"+appTitle);
    }
    m_someattribute = infoList.join(",");
    emit mdPathOtherChanged(m_someattribute);
}

void Bridge::getThemeColor(QString color){
    m_someattribute = color;
    emit highColorChange(m_someattribute);
}

void Bridge::getTheme(QString theme){
    m_someattribute = theme;
    emit themeChange(m_someattribute);
}

void Bridge::getMdContent(QString attrStr)
{
    qDebug() << Q_FUNC_INFO;
     // 将app、path、content传递过过去
    QString appPath = this->js_getIndexMdFilePathOther(attrStr);
    QString appContent = this->js_getIndexMdFileContent(appPath);
    QString appDate = this->js_getFileSystemInformation(appPath);
    m_someattribute = attrStr+"|"+appPath+"|"+appDate+"|"+appContent;
    emit showBackbutton();
    emit mdContentChanged(m_someattribute);
}

QStringList Bridge::js_getIntoFilename()
{
    qDebug() << Q_FUNC_INFO;
    QStringList test=this->getDirAndPng();
    qDebug() << test;
    return test;
}

QStringList Bridge::getDirAndPng()
{
    qDebug() << Q_FUNC_INFO;
    QStringList list;
    QStringList pathList; // 假设这是外部传入的路径列表

    // 逻辑1：判断 pathList 是否为空
    if (pathList.isEmpty()) {
        // 为空时使用默认路径
        processSinglePath(LOCAL_FILE_PATH_UBUNTUKYLIN, list);
    } else {
        // 非空时遍历所有路径
        for (const QString &currentPath : pathList) {
            processSinglePath(currentPath, list);
        }
    }
    return list;
}

// 辅助函数：处理单个路径
void Bridge::processSinglePath(const QString &path, QStringList &outputList) {
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << "Path" << path << "does not exist!";
        return;
    }

    // 获取目录列表并排序
    QStringList dirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    dirList.sort();

    // 遍历子目录
    for (const QString &subDir : dirList) {
        QDir subDirPath(dir.filePath(subDir)); // 构建完整子目录路径
        QStringList pngFiles = subDirPath.entryList({"*.png"}, QDir::Files);

        // 将子目录名与所有 PNG 文件组合
        for (const QString &pngFile : pngFiles) {
            outputList << subDir + "|" + pngFile;
        }
    }
}

QString Bridge::system_name()
{
    qDebug() << "system_name";
    QFile system (SYSTEM_FILE);
    if(!system.exists())
    {
        return "";
    }
    if (!system.open(QIODevice::ReadOnly))
    {
        return "";
    }
    QString str = system.readLine();
    QString name = str.section("\"",1,1);
    qDebug() << name;
    return name;
    //return "Ubuntu Kylin";
}

QString Bridge::js_getIndexMdFileInfo(QString appName){
    this->getMdPathOther(appName);
    return "";
}

QString Bridge::js_getPageContent(QString appName){
    this->getMdContent(appName);
    return "";
}

QString Bridge::js_getIndexMdFilePathOther(QString appName) {
    
    // 若 pathList 为空，使用默认路径
    if (pathList.isEmpty()) {
        QString defaultPath = LOCAL_FILE_PATH_UBUNTUKYLIN + appName + "/" + gLang + "/index.md";
        return defaultPath;
    }

    // 遍历 pathList，生成并验证路径
    for (QStringList::const_iterator it = pathList.begin(); it != pathList.end(); ++it) {
        QString fullPath = *it + "/" + appName + "/" + gLang + "/index.md";
        if (QFile::exists(fullPath)) {
            return fullPath;
        }
    }

    return LOCAL_FILE_PATH_UBUNTUKYLIN + appName + "/" + gLang + "/index.md";;
}

QString Bridge::js_getIndexMdFileContent(QString IndexMdFilePath)
{
    qDebug() << Q_FUNC_INFO << IndexMdFilePath;
    QFile file (IndexMdFilePath);
    if(!file.exists())
        return "";
    if(!file.open(QIODevice::ReadOnly))
        return "";
    return file.readAll();
}

QString Bridge::js_getIndexMdFileTitle(QString IndexMdFilePath)
{
    qDebug() << Q_FUNC_INFO << IndexMdFilePath;
    QFile file (IndexMdFilePath);
    if(!file.exists())
        return "";
    if(!file.open(QIODevice::ReadOnly))
        return "";
    QString title = file.readLine();
    title = title.mid(1,title.length());
    return title.trimmed();
}

void Bridge::js_openUrl(QUrl url)
{
    if(url.toString().contains("http")||url.toString().contains("mailto")||url.toString().contains("https"))
        QDesktopServices::openUrl(url);
}

QString Bridge::js_getFileSystemInformation(QString filePath)
{
    if (filePath.isNull())
        return "";

     QFileInfo fileinfo(filePath);
    if (fileinfo.exists()) {
        return fileinfo.lastModified().toString("yyyy年MM月dd日");
    } else {
        qDebug() << Q_FUNC_INFO << "This File is not exist !!!";
        return "";
    }
}

void Bridge::js_changeNavHighlight(){
    settings->get("themeColor").toString();
    QPalette pal = qApp->palette();
    QColor highlight = pal.color(QPalette::Active, QPalette::Highlight);
    this->getThemeColor(converRGB2HexStr(highlight));
}

void Bridge::js_getIconThemeName(){
    QString iconTheme = settings->get("iconThemeName").toString();
    this->getTheme(iconTheme);
}

void Bridge::slot_SettingsChange(const QString &key)
{
    qDebug() << Q_FUNC_INFO << key << settings->get("styleName").toString();
    if(key == "styleName"){
        QPalette palette;
        if(settings->get("styleName").toString() == "ukui-black" || settings->get("styleName").toString() == "ukui-dark"){
            palette.setBrush(QPalette::Background, QBrush(QColor(38,38,38)));
            palette.setBrush(QPalette::Text, QBrush(QColor(Qt::white)));
//            this->setPalette(palette);
        }else{
            palette.setBrush(QPalette::Background, QBrush(QColor(Qt::white)));
            palette.setBrush(QPalette::Text, QBrush(QColor(29,29,31)));
//            this->setPalette(palette);
        }
    }
    if ("iconThemeName" == key) {
//        m_pIconLabel->setPixmap(QIcon::fromTheme("kylin-user-guide").pixmap(24,24));
        QString iconTheme = settings->get("iconThemeName").toString();
        QString themeJs = QString("switchTheme(\"%1\");").arg(iconTheme);
         if(!iconTheme.isNull()){
         }
    }

    if("systemFont" == key || "systemFontSize" == key ){
//        QFont font = this->font();
//        for (auto widget: qApp->allWidgets()) {
            qDebug() << Q_FUNC_INFO;
//            widget->setFont(font);
//        }
//        m_pTitleLabel->adjustSize();
    }

    if("themeColor" == key){
        js_changeNavHighlight();
    }
}

QString Bridge::converRGB2HexStr(QColor _rgb)
{
	QString redStr = QString("%1").arg(_rgb.red(),2,16,QChar('0'));
	QString greenStr = QString("%1").arg(_rgb.green(),2,16,QChar('0'));
	QString blueStr = QString("%1").arg(_rgb.blue(),2,16,QChar('0'));
    QString hexStr = "#"+redStr+greenStr+blueStr;
	return hexStr;
}

void Bridge::js_executeCommand(QString cmd ,QStringList args)
{
    qDebug() << Q_FUNC_INFO << cmd;
    qDebug() << Q_FUNC_INFO << args;
    QProcess::startDetached(cmd,args);
}

void Bridge::js_buriedPoint(QString appName)
{
    qDebug() << Q_FUNC_INFO;
    // 埋点数据

    QString app = "kylin-user-guide";
    char *appCstr = strdup(app.toUtf8().constData());

    QString action = "user_guide_search_event";
    char *actionCstr = strdup(action.toUtf8().constData());

    KCustomProperty property[1];
    QString appKey= "application";
    char *keyCstr = strdup(appKey.toUtf8().constData());
    property[0].key = keyCstr;
    char *valueCstr = strdup(appName.toUtf8().constData());
    property[0].value = valueCstr;

    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
    kdk_dia_append_custom_property(node,property,1);
    kdk_dia_upload_default(node,actionCstr,appCstr);
    kdk_dia_data_free(node);
}
