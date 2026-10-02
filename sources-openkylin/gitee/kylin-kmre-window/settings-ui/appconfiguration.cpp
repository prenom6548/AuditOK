/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "appconfiguration.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QFileSystemWatcher>
#include "utils.h"

AppConfiguration *AppConfiguration::getInstance()
{
    static AppConfiguration appConfiguration;
    return &appConfiguration;
}

AppConfiguration::AppConfiguration(/* args */)
    : mAppConfigurationFile(QDir::homePath() + "/.config/kmre/app_configuration.xml")
    , mFileWatcher(new QFileSystemWatcher(this))
{
    reLoadAppConfigution();

    if (mFileWatcher->addPath(mAppConfigurationFile)) {
        connect(mFileWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(slotConfigFileChanged(const QString &)));
    }
    else {
        qWarning() << "Add config file:" << mAppConfigurationFile << " to file watcher failed!";
    }
}

AppConfiguration::~AppConfiguration()
{
}

void AppConfiguration::getAppListFromXml(const QString &filePath, QStringList &appList, 
                                    const QString rootTag, const QString childTag)
{
    appList.clear();
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString errorStr;
        int errorLine;
        int errorColumn;

        QDomDocument doc;
        if (doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn)) {
            QDomElement root = doc.documentElement();
            if (root.tagName() == rootTag) {
                QDomNode child = root.firstChild();
                while (!child.isNull()) {
                    if (child.toElement().tagName() == childTag) {
                        appList << child.toElement().text();
                    }
                    child = child.nextSibling();
                }
            }
        }
        else {
            qWarning() << "Parse error at line: " << errorLine << ", column: " << errorColumn << ", info: " << errorStr;
        }
    }
    else {
        qWarning() << "Open file: " << filePath << " failed!";
    }

    // qDebug() << "Get app list from: " << filePath;
    // for (auto app : appList) {
    //     qDebug() << app;
    // }
}

void AppConfiguration::reLoadAppConfigution()
{
    if (!kmre::utils::checkAndCreateFile(mAppConfigurationFile, 0666)) {
        return;
    }

    auto data = kmre::utils::tryReadFile(mAppConfigurationFile, true, 100);

    if (data.size() > 0) {
        QString errorStr;
        int errorLine;
        int errorColumn;

        std::lock_guard<std::mutex> lk(mAppConfigurationDocLk);
        mAppConfigurationDoc.clear();

        data.replace('+', ':');// char '+' is not allowed in xml
        if (mAppConfigurationDoc.setContent(data, false, &errorStr, &errorLine, &errorColumn)) {
            QDomElement apps = mAppConfigurationDoc.documentElement();
            if (apps.tagName() != "apps") {
                qWarning() << "Can't find node 'apps' in app config file!";
                mAppConfigurationDoc.clear();
            }
        }
        else {
            qWarning() << "Parse error in app config file!";
        }
    }
    else {
        qWarning() << "Open app config file failed!";
    }
}

AppConfiguration::AppConfig AppConfiguration::getAppConfig(const QString& pkgName)
{
    AppConfig appConfig = {
        .bootFullScreen = false,
        .trayEnabled = false,
    };

    std::lock_guard<std::mutex> lk(mAppConfigurationDocLk);

    if (!mAppConfigurationDoc.isNull()) {
        QDomElement root = mAppConfigurationDoc.documentElement();
        if (root.tagName() == "apps") {
            QString tmpPkgName = pkgName;
            tmpPkgName.replace('+', ':');// char '+' is not allowed in xml

            QDomNodeList apps = root.elementsByTagName(tmpPkgName);
            if (apps.size() > 0) {
                QDomElement app = apps.at(0).toElement();

                QDomNode child = app.firstChild();
                while (!child.isNull()) {
                    if (child.toElement().tagName() == "BootFullScreen") {
                        QString tmpStr = child.toElement().text();
                        appConfig.bootFullScreen = (tmpStr.toLower() == "true");
                    }
                    else if (child.toElement().tagName() == "Tray") {
                        QString tmpStr = child.toElement().text();
                        appConfig.trayEnabled = (tmpStr.toLower() == "true");
                    }

                    child = child.nextSibling();
                }
            }
        }
    }

    qDebug() << "bootFullScreen = " << appConfig.bootFullScreen; 
    qDebug() << "trayEnabled = " << appConfig.trayEnabled; 
    return appConfig;
}

QStringList AppConfiguration::getAppListByConfig(const QString& cfgName, bool value)
{
    QStringList appList;    
    std::lock_guard<std::mutex> lk(mAppConfigurationDocLk);

    if (!mAppConfigurationDoc.isNull()) {
        QDomElement root = mAppConfigurationDoc.documentElement();
        if (root.tagName() == "apps") {
            QDomNode pkgNode = root.firstChild();
            while (!pkgNode.isNull()) {
                QString pkgName = pkgNode.toElement().tagName();
                pkgName.replace(':', '+');// char '+' is not allowed in xml

                QDomNode child = pkgNode.firstChild();
                while (!child.isNull()) {
                    if (child.toElement().tagName() == cfgName) {
                        QString valueStr = child.toElement().text();
                        if (value == (valueStr.toLower() == "true")) {
                            appList << pkgName;
                        }
                    }

                    child = child.nextSibling();
                }

                pkgNode = pkgNode.nextSibling();
            }
        }
    }

    qDebug() << "[" << __func__ << "] '" << cfgName << "' appList = " << appList; 
    return std::move(appList);
}

bool AppConfiguration::setAppConfig(const QString& pkgName, const AppConfig& appConfig)
{
    // set app config
    QDomElement apps;

    std::lock_guard<std::mutex> lk(mAppConfigurationDocLk);

    if (mAppConfigurationDoc.isNull()) {
        QDomProcessingInstruction instruction;
        instruction = mAppConfigurationDoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
        mAppConfigurationDoc.appendChild(instruction);

        apps = mAppConfigurationDoc.createElement("apps");
        mAppConfigurationDoc.appendChild(apps);
    }
    else {
        apps = mAppConfigurationDoc.documentElement();
    }

    if (apps.tagName() == "apps") {
        QDomElement app;
        QString tmpPkgName = pkgName;
        tmpPkgName.replace('+', ':');// char '+' is not allowed in xml

        QDomNodeList appsList = apps.elementsByTagName(tmpPkgName);

        if (appsList.size() <= 0) {
            app = mAppConfigurationDoc.createElement(tmpPkgName);
            apps.appendChild(app);
        }
        else {
            app = appsList.at(0).toElement();
        }

        typedef struct {
            QString cfgName;
            QString cfgValue;
        }AppConfigPair;

        std::vector<AppConfigPair> AppConfigurationList = {
            {"BootFullScreen",  appConfig.bootFullScreen ? "true" : "false"},
            {"Tray",            appConfig.trayEnabled ? "true" : "false"},
        };

        for (const auto &cfg : AppConfigurationList) {
            QDomNodeList nodeList = app.elementsByTagName(cfg.cfgName);
            if (nodeList.size() > 0) {
                QDomNode cfgNode = nodeList.at(0);
                QDomNode oldNode = cfgNode.firstChild();
                cfgNode.firstChild().setNodeValue(cfg.cfgValue);
                QDomNode newNode = cfgNode.firstChild();
                cfgNode.replaceChild(newNode, oldNode);
            }
            else {
                QDomElement cfgElement = mAppConfigurationDoc.createElement(cfg.cfgName);
                QDomText value = mAppConfigurationDoc.createTextNode(cfg.cfgValue);
                cfgElement.appendChild(value);
                app.appendChild(cfgElement);
            }
        }
    }

    // save configuration to file
    mFileWatcher->removePath(mAppConfigurationFile);
    bool ret = kmre::utils::tryWriteFile(mAppConfigurationFile, mAppConfigurationDoc.toByteArray(), true, 100);
    mFileWatcher->addPath(mAppConfigurationFile);

    if (ret) {
        qWarning() << "Set app configuration success.";
    }
    else {
        qWarning() << "Write app configuration file: " << mAppConfigurationFile << " failed!";
    }
    return ret;
}

bool AppConfiguration::setAppBootFullScreen(const QString& pkgName, bool fullScreen)
{
    AppConfig appConfig = getAppConfig(pkgName);

    if (appConfig.bootFullScreen != fullScreen) {
        appConfig.bootFullScreen = fullScreen;
        return setAppConfig(pkgName, appConfig);
    }

    return true;
}

bool AppConfiguration::setAppTray(const QString& pkgName, bool enable)
{
    AppConfig appConfig = getAppConfig(pkgName);

    if (appConfig.trayEnabled != enable) {
        appConfig.trayEnabled = enable;
        return setAppConfig(pkgName, appConfig);
    }
    
    return true;
}

void AppConfiguration::slotConfigFileChanged(const QString &path)
{
    qInfo() << "Config file:" << path << " changed!";

    if (path == mAppConfigurationFile) {
        reLoadAppConfigution();
    }

    if (!mFileWatcher->files().contains(path)) {
        if (!mFileWatcher->addPath(path)) {
            qWarning() << "re-add file" << path << " to file watcher failed!";
        }
    }
}