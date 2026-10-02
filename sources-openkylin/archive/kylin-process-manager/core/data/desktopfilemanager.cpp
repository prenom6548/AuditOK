/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "desktopfilemanager.h"
#include "loghelper.h"
#include "processinfohelper.h"
#include "waylanddisplay.h"

#include <KDesktopFile>
#include <QDebug>
#include <QDir>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QtConcurrent>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <QFileInfo>

#define XDG_AUTO_START_PATH "/etc/xdg/autostart"
#define KWRE_CMD_LINE_PATH "/opt/kylin-wine/app"
#define KARE_CMD_LINE_PATH "/opt/kare/usr/bin"
#define ETC_UKUI_SEARCH_CONF "/etc/ukui/ukui-search/application-dirs.conf"

#define VAR_LIB_SNAPD_DESKTOP_APPLICATIONS_PATH "/var/lib/snapd/desktop/applications"
#define VAR_LIB_FLATPAK_EXPORTS_SHARE_APPLICATIONS_PATH "/var/lib/flatpak/exports/share/applications"

#define KWRE_TYPE "kylin-wine"
#define KARE_TYPE "kare"

#define KARE_BIN "/usr/bin/kare"

DesktopFileManager::DesktopFileManager(QObject *parent)
    : QObject{parent}
{
    initAllDirDesktopSize();
    loadDesktopFiles();
    loadKwreCmdLineFiles();
    loadKareCmdLineFiles();
}

void DesktopFileManager::initAllDirDesktopSize() 
{
    auto desktopPaths = desktopFilePaths();
    for(auto path : desktopPaths) {
        int cnt = getFileCount(path);
        qDebug() << "deskttopPath: " << path << " file size: " << cnt;
        m_desktopPathFileCnt[path] = cnt;
    }
}

bool DesktopFileManager::hasDirDesktopfileSizeChanged()
{
    auto desktopPaths = desktopFilePaths();
    bool changed = false;
    for(auto path : desktopPaths) {
        int cnt = getFileCount(path);
        
        qDebug() << "deskttopPath: " << path << " file size: " << cnt;
        if(cnt != m_desktopPathFileCnt[path]) {
            PRINT_NOTICE("DesktopFileManager::hasDirDesktopfileSizeChanged(): desktoppath: %s\n", path.toStdString().c_str());
            changed = true;
            m_desktopPathFileCnt[path] = cnt;
        }
    }
    return changed;
}

int DesktopFileManager::getFileCount(const QString &path) {
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << "路径不存在:" << path;
        return 0;
    }

    // 设置过滤条件：只包含文件，不包含目录
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    // 获取文件列表
    QFileInfoList fileList = dir.entryInfoList();

    // 返回文件个数
    return fileList.size();
}

std::string DesktopFileManager::syncGetDesktopFileByPid(int pid, bool needUpDateDesktopTable)
{
    if (isKylinOsManagerTool(pid)) {
        return {};
    }

    // 确保desktop文件缓存为最新。启动process-manager后新安装的应用不在desktopFile缓存中，会导致找不到desktop文件
    if(needUpDateDesktopTable && hasDirDesktopfileSizeChanged()) {

        loadDesktopFilesSync();
        loadKwreCmdLineFilesSync();
        loadKareCmdLineFilesSync();
        // loadDesktopFiles();
        // loadKwreCmdLineFiles();
        // loadKareCmdLineFiles();
    }

    QString desktopFile = findDesktopFileFromLocalCache(pid);
    if (!desktopFile.isEmpty()) {
        return desktopFile.toStdString();
    }

    return std::string();
}

std::string DesktopFileManager::syncGetDesktopFileByAppId(const std::string &appId)
{
    QString desktopFile = findDesktopFileByAppId(QString::fromStdString(appId));
    if (desktopFile.isEmpty()) {
        return {};
    }
    return desktopFile.toStdString();
}

std::vector<std::string> DesktopFileManager::desktopFilesWithName(const std::string &desktopName)
{
    if (m_loadDesktopFilesFuture.isRunning()) {
        m_loadDesktopFilesFuture.waitForFinished();
    }

    std::vector<std::string> files;
    auto filePath = desktopFilePath(QString::fromStdString(desktopName));
    if (filePath.isEmpty()) {
        return files;
    }

    auto desktopFiles = m_desktopExecs.keys();
    for (auto &desktopFile : qAsConst(desktopFiles)) {
        if (desktopFile.split("/").last() == QString::fromStdString(desktopName)) {
            files.emplace_back(desktopFile.toStdString());
        }
    }

    return files;
}

std::string DesktopFileManager::findDesktopFileByTitle1(const std::string &wid, const int &pid)
{
    const auto windows = waylandDisplay()->getWindows();
    if(windows.find(wid) == windows.end()) return std::string();
    QString title(windows.at(wid)->title.c_str());
    title = title.trimmed();
    QString desktopFile;
    QString cmdlineResult = QString::fromStdString(process_info_helper::cmdline(pid));
    cmdlineResult = cmdlineResult.trimmed();
    auto desktopFileEqualName = [title, cmdlineResult](const QMap<QString, QStringList> &namesMap, const DesktopExecMap &desktopsExec) {
        auto it = namesMap.constBegin();
        while (it != namesMap.constEnd()) {
            auto desktopFile = it.key();
            QStringList names = it.value();
            if (names.contains(title)) {

                QString exec;
                QStringList realCmds;
                std::tie(exec, realCmds) = desktopsExec[desktopFile];
                QString execPath = exec.left(exec.lastIndexOf("/"));
                if (cmdlineResult.contains(execPath)) {
                    return desktopFile;
                }
            }
            ++it;
        }
        return QString();
    };

    desktopFile = desktopFileEqualName(m_desktopNames, m_desktopExecs);
    if (!desktopFile.isEmpty()) {
        return desktopFile.toStdString();
    }
    return std::string();
}

void DesktopFileManager::loadDesktopFiles()
{
    auto desktopPaths = desktopFilePaths();
    auto desktopFileExecFunction = [desktopPaths, this]() {
        for (const auto &desktopPath : desktopPaths) {
            updateDesktopExecsInfo(desktopPath);
            updateDesktopNamesInfo(desktopPath);
            updateDesktopStartupWmClassInfo(desktopPath);
        }
    };
    m_loadDesktopFilesFuture = QtConcurrent::run(desktopFileExecFunction);
}

void DesktopFileManager::loadDesktopFilesSync()
{
    auto desktopPaths = desktopFilePaths();
    for (const auto &desktopPath : desktopPaths) {
        updateDesktopExecsInfo(desktopPath);
        updateDesktopNamesInfo(desktopPath);
        updateDesktopStartupWmClassInfo(desktopPath);
    }
}

void DesktopFileManager::loadKwreCmdLineFiles()
{
    QDir dir(KWRE_CMD_LINE_PATH);
    if (!dir.exists()) {
        return;
    }
    QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot); // 只获取子目录

    auto dissectKwreCmdLineFunction = [subDirs, this]() {
        for (auto &subDirItem : qAsConst(subDirs)) {
            updateKwreCmdlinesInfo(subDirItem, QString::fromStdString(KWRE_CMD_LINE_PATH));
        }
    };

    m_loadKwreCmdLineFilesFuture = QtConcurrent::run(dissectKwreCmdLineFunction);
}

void DesktopFileManager::loadKwreCmdLineFilesSync()
{
    QDir dir(KWRE_CMD_LINE_PATH);
    if (!dir.exists()) {
        return;
    }
    QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot); // 只获取子目录

    for (auto &subDirItem : qAsConst(subDirs)) {
        updateKwreCmdlinesInfo(subDirItem, QString::fromStdString(KWRE_CMD_LINE_PATH));
    }
}

void DesktopFileManager::loadKareCmdLineFiles()
{
    QDir dir(KARE_CMD_LINE_PATH);
    if (!dir.exists()) {
        return;
    }

    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    auto dissectKareCmdLineFunction = [files, this]() {
        for (auto &programFile : qAsConst(files)) {
            // if (programFile.contains('.')) { // 如果文件名不含 '.'，则保留
            //    continue;
            // }
            QString wholeProgramFilePath = QString::fromStdString(KARE_CMD_LINE_PATH) + "/" + programFile;
            QStringList cmdLineList = getCmdLineByProgramFile(wholeProgramFilePath, QString::fromStdString(KARE_TYPE));
            if (!cmdLineList.isEmpty()) {
                m_kareCmdLines[wholeProgramFilePath] = cmdLineList;
            }
        }
    };

    m_loadKareCmdLineFilesFuture = QtConcurrent::run(dissectKareCmdLineFunction);
}

void DesktopFileManager::loadKareCmdLineFilesSync()
{
    QDir dir(KARE_CMD_LINE_PATH);
    if (!dir.exists()) {
        return;
    }

    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (auto &programFile : qAsConst(files)) {
        QString wholeProgramFilePath = QString::fromStdString(KARE_CMD_LINE_PATH) + "/" + programFile;
        QStringList cmdLineList = getCmdLineByProgramFile(wholeProgramFilePath, QString::fromStdString(KARE_TYPE));
        if (!cmdLineList.isEmpty()) {
            m_kareCmdLines[wholeProgramFilePath] = cmdLineList;
        }
    }
}

QStringList DesktopFileManager::desktopFilePaths() const
{
    QStringList desktopPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    desktopPaths.append(XDG_AUTO_START_PATH);
    desktopPaths.append(VAR_LIB_FLATPAK_EXPORTS_SHARE_APPLICATIONS_PATH);
    desktopPaths.append(VAR_LIB_SNAPD_DESKTOP_APPLICATIONS_PATH);
    // QFile file(ETC_UKUI_SEARCH_CONF);
    //  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    //      qDebug() << "Cannot open file for reading";
    //      return desktopPaths;
    //  }
    //  QTextStream in(&file);
    //  while (!in.atEnd()) {
    //      QString line = in.readLine();
    //      QStringList desktopDirs = line.split(",");
    //      for(auto &dirItem : desktopDirs) {
    //          if(!desktopPaths.contains(dirItem)) {
    //              desktopPaths.append(dirItem);
    //          }
    //      }
    //  }
    return desktopPaths;
}

QStringList DesktopFileManager::getCmdLineByProgramFile(const QString &programFilePath, const QString &type)
{
    QStringList cmdLineList;
    if (programFilePath.isEmpty()) {
        return QStringList();
    }
    QFile file(programFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        PRINT_DBG("open file failed\n");
        return QStringList();
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains(type)) {
            line.remove(QRegExp("\\\\"));
            line.remove(QRegExp("\""));
            line.remove(QRegExp("\\s+"));
            cmdLineList << line;
        }
    }

    return cmdLineList;
}

void DesktopFileManager::updateKwreCmdlinesInfo(const QString &subPath, const QString &path)
{
    QDir dir(path + "/" + subPath);
    if (!dir.exists()) {
        return;
    }
    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (auto &programFile : qAsConst(files)) {
        if (programFile.contains('.')) { // 如果文件名不含 '.'，则保留
            continue;
        }
        QString wholeProgramFilePath = path + "/" + subPath + "/" + programFile;
        QStringList cmdLineList = getCmdLineByProgramFile(wholeProgramFilePath, QString::fromStdString(KWRE_TYPE));
        if (!cmdLineList.isEmpty()) {
            m_kwreCmdLines[wholeProgramFilePath] = cmdLineList;
        }
    }
}

void DesktopFileManager::updateKareCmdlinesInfo(const QString &subPath, const QString &path)
{
    QDir dir(path + "/" + subPath);
    if (!dir.exists()) {
        return;
    }
    QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (auto &programFile : qAsConst(files)) {
        // if (programFile.contains('.')) { // 如果文件名不含 '.'，则保留
        //    continue;
        // }
        QString wholeProgramFilePath = path + "/" + subPath + "/" + programFile;
        QStringList cmdLineList = getCmdLineByProgramFile(wholeProgramFilePath, KARE_TYPE);
        if (!cmdLineList.isEmpty()) {
            m_kareCmdLines[wholeProgramFilePath] = cmdLineList;
        }
    }
}

QStringList DesktopFileManager::getRealCmdByExec(const QString &exec) const
{
    QStringList cmdLineList;
    if (exec.isEmpty()) {
        return QStringList();
    }
    QFile file(exec);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        PRINT_NOTICE("open file failed\n");
        return QStringList();
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QString lineResult = line.remove("$@");
        lineResult = lineResult.remove(QRegExp("\\s+"));
        if (lineResult.startsWith("exec")) {
            lineResult.remove(QRegExp("^exec"));
            lineResult.remove(QRegExp("\""));
            cmdLineList << lineResult;
        }
    }
    return cmdLineList;
}

QStringList DesktopFileManager::getKareRealCmd(const QString &exec) const
{
    QStringList cmdLineList;
    if (exec.isEmpty()) {
        return QStringList();
    }
    QFile file(exec);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        PRINT_NOTICE("open file failed\n");
        return QStringList();
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QString lineResult = line.remove("$@");
        lineResult = lineResult.remove(QRegExp("\\s+"));
        if (lineResult.contains(QString::fromStdString(KARE_TYPE))) {
            lineResult.remove(QRegExp("\\\\"));
            lineResult.remove(QRegExp("\""));
            lineResult.remove(QRegExp("\\s+"));
            cmdLineList << line;
        }
    }
    return cmdLineList;
}

void DesktopFileManager::updateDesktopExecsInfo(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    QStringList desktopFiles = dir.entryList(QStringList() << "*.desktop");
    for (auto &desktopFileName : qAsConst(desktopFiles)) {
        QString deskopFile = path + "/" + desktopFileName;
        QString exec;
        QStringList args;
        std::tie(exec, args) = getExecByDesktopFile(deskopFile);
        {
            if (exec.isEmpty()) {
                PRINT_DBG("The Exec in %s desktop file is empty.\n", desktopFileName.toStdString().c_str());
                continue;
            }
            QMutexLocker locker(&m_mutex);
            if (!m_desktopExecs.contains(deskopFile)) {
                m_desktopExecs[deskopFile] = {exec, args};
            }
        }

        {
            if(process_info_helper::isShellScript(exec) && exec.startsWith("/opt")) {
                PRINT_DBG("The Exec in %s desktop file is script.\n", desktopFileName.toStdString().c_str());
                QStringList cmdLst = getRealCmdByExec(exec);
                if (!cmdLst.isEmpty()) {
                    m_desktopExecRealCmds[deskopFile] = {exec, cmdLst};
                }
            }
        }

        {
            if (exec == KARE_BIN) {
                for (auto arg : args) {
                    if (process_info_helper::isShellScript(arg)) {
                        PRINT_DBG("The Exec in %s desktop file is script. for kare\n", desktopFileName.toStdString().c_str());
                        QStringList cmdLst = getKareRealCmd(arg);
                        if (!cmdLst.isEmpty()) {
                            m_kareCmdLines[deskopFile] = cmdLst;
                            PRINT_DBG("updateDesktopExecsInfo desktopfile %s, cmdLst %s\n", deskopFile.toStdString().c_str(), cmdLst.join(",").toStdString().c_str());
                        }
                    }
                }
            }
        }
    }
}

void DesktopFileManager::updateDesktopNamesInfo(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    QStringList desktopFiles = dir.entryList(QStringList() << "*.desktop");
    for (auto &desktopFileName : qAsConst(desktopFiles)) {
        QString deskopFile = path + "/" + desktopFileName;
        QStringList desktopNames;
        desktopNames = getAllNamesByDesktopFile(deskopFile);
        {
            if (desktopNames.isEmpty()) {
                PRINT_DBG("The Name in %s desktop file is empty.\n", desktopFileName.toStdString().c_str());
                continue;
            }

            QMutexLocker locker(&m_mutex);
            if (!m_desktopNames.contains(deskopFile)) {
                m_desktopNames[deskopFile] = desktopNames;
            }
        }
    }
}

void DesktopFileManager::updateDesktopStartupWmClassInfo(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    QStringList desktopFiles = dir.entryList(QStringList() << "*.desktop");
    for (auto &desktopFileName : qAsConst(desktopFiles)) {
        QString deskopFile = path + "/" + desktopFileName;

        QString startupWmClass = getStartupWmClass(deskopFile);
        {
            if (startupWmClass.isEmpty()) {
                PRINT_DBG("The startupWmClass in %s desktop file is empty.\n", startupWmClass.toStdString().c_str());
                continue;
            }

            QMutexLocker locker(&m_mutex);
            if (!m_desktopStartupWmClass.contains(deskopFile)) {
                m_desktopStartupWmClass[deskopFile] = startupWmClass;
            }
        }
    }
}

std::tuple<QString, QStringList> DesktopFileManager::getExecByDesktopFile(const QString &desktopFile) const
{
    if (desktopFile.isEmpty()) {
        return std::make_tuple(QString(), QStringList());
    }
    QString _desktopFilePath = desktopFilePath(desktopFile);
    KDesktopFile desktop(_desktopFilePath);
    QString exec = desktop.entryMap("Desktop Entry").value("Exec");
    if (exec.isEmpty()) {
        return {QString(), QStringList()};
    }

    exec.remove(QRegExp("%."));
    exec.remove(QRegExp("^\""));
    exec.remove(QRegExp(" *$"));
    exec.remove(QRegExp("\""));

    QStringList execArgs = exec.split(" ");
    if (execArgs.count() > 0) {
        exec = execArgs.first();
    }

    return {exec, execArgs};
}

QStringList DesktopFileManager::getAllNamesByDesktopFile(const QString &desktopFile) const
{
    if (desktopFile.isEmpty()) {
        return QStringList();
    }
    QStringList desktopNames;
    QString _desktopFilePath = desktopFilePath(desktopFile);
    KDesktopFile desktop(_desktopFilePath);
    QMap<QString, QString> desktopEntryMap = desktop.entryMap("Desktop Entry");
    QMapIterator<QString, QString> it(desktopEntryMap);
    while (it.hasNext()) {
        it.next();
        if (it.key().contains("Name")) {
            QString name = it.value();
            name = name.remove(" ");
            desktopNames << name;
        }
    }
    // qDebug() << "desktop: " << _desktopFilePath;
    // qDebug() << "Names: " << desktopNames;
    return desktopNames;
}

QString DesktopFileManager::getStartupWmClass(const QString &desktopFile) const
{
    if (desktopFile.isEmpty()) {
        return QString();
    }
    QString _desktopFilePath = desktopFilePath(desktopFile);
    KDesktopFile desktop(_desktopFilePath);
    QString startupWmClass = desktop.entryMap("Desktop Entry").value("StartupWMClass");
    qDebug() << "desktopFile: " << desktopFile << "startupWmClass: " << startupWmClass;
    return startupWmClass;
}

QString DesktopFileManager::desktopFilePath(const QString &desktopFileName) const
{
    if (desktopFileName.isEmpty()) {
        return QString();
    }
    QFile desktopFile(desktopFileName);
    if (desktopFile.exists()) {
        return desktopFileName;
    }

    auto desktopPaths = desktopFilePaths();
    for (auto &path : qAsConst(desktopPaths)) {
        if (desktopFileName.contains(path)) {
            return desktopFileName;
        } else if (QFile::exists(path + "/" + desktopFileName)) {
            return path + "/" + desktopFileName;
        }
    }
    return QString();
}

QString DesktopFileManager::findDesktopFileFromLocalCache(const int &pid)
{
    QString cmdline = QString::fromStdString(process_info_helper::cmdline(pid));
    if (cmdline.isEmpty()) {
        PRINT_DBG("DesktopFileManager::findDesktopFileFromLocalCache: cmdline is empty!");
        return QString();
    }
    if (cmdline.contains(polkitWindowPidCmdline)) {
        return QString();
    }
    if (m_loadDesktopFilesFuture.isRunning()) {
        m_loadDesktopFilesFuture.waitForFinished();
    }

    if (m_loadKwreCmdLineFilesFuture.isRunning()) {
        m_loadKwreCmdLineFilesFuture.waitForFinished();
    }

    if (m_loadKareCmdLineFilesFuture.isRunning()) {
        m_loadKareCmdLineFilesFuture.waitForFinished();
    }

    QString desktopFile;
    PRINT_DBG("findDesktopFileFromLocalCache pid: %d, cmdline: %s\n", pid, cmdline.toStdString().c_str());
    // 尝试从命令行猜测 App ID
    if (const auto args = cmdline.split(' ', Qt::SkipEmptyParts); !args.isEmpty()) {
        const auto appId = args.first().section('/', -1); // 获取路径的最后一部分
        if (!appId.isEmpty()) {
            if (const auto desktopFile = findDesktopFileByAppId(appId); !desktopFile.isEmpty()) {
                return desktopFile;
            }
        }
    }

    if (cmdline.startsWith(kmreAppMainWindow)) {
        desktopFile = findKmreFullDesktopFileByCmdline(cmdline);
    }
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    desktopFile = findDesktopFileFromPid(pid);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    desktopFile = findDesktopFileByEqualExec(cmdline);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    QString binaryPath = binaryPathFromCmdline(cmdline);
    desktopFile = findDesktopFileByEqualExec(binaryPath);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    desktopFile = findDesktopFileByEqualStartArg(cmdline);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    desktopFile = findDesktopFileByContainsStartArgs(cmdline);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }

    desktopFile = findDesktopFileByContainsExec(binaryPath);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }

    QString kwreProgramInstallPath = findKwreProgramInstallPath(cmdline); // kwre
    if (!kwreProgramInstallPath.isEmpty()) {
        desktopFile = findDesktopFileByEqualExec(kwreProgramInstallPath);
        if (!desktopFile.isEmpty()) {
            return desktopFile;
        }
    }

    QString kareProgramInstallPath = findKareProgramInstallPath(cmdline); // kare
    if (!kareProgramInstallPath.isEmpty()) {
        if (kareProgramInstallPath.endsWith(".desktop")) {
            return kareProgramInstallPath;
        }
        desktopFile = findDesktopFileByEqualExec(kareProgramInstallPath);
        if (!desktopFile.isEmpty()) {
            return desktopFile;
        }
    }

    desktopFile = findDesktopFileByRealCmd(cmdline);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }

    int ppid = process_info_helper::parentPid(pid);
    if (process_info_helper::cmdline(ppid) == "bash") {
        return QString();
    }
    return ppid <= 1 ? QString() : findDesktopFileFromLocalCache(ppid);
}

QString DesktopFileManager::findDesktopFileByAppId(const QString &appId) const
{
    if (appId.isEmpty()) {
        return QString();
    }

    QString appId_ = appId;
    if (m_AppIdToDesktopFile.contains(appId)) {
        appId_ = m_AppIdToDesktopFile[appId];
    }

    auto it = m_desktopExecs.constBegin();
    while (it != m_desktopExecs.constEnd()) {
        //KDesktopFile desktop(it.key());
        auto itWmClass = m_desktopStartupWmClass.find(it.key());
        QString startupWmClass;
        if(itWmClass != m_desktopStartupWmClass.end()) {
            startupWmClass = itWmClass.value();
            //qDebug() << "DesktopFileManager::findDesktopFileByAppId: " << "startupWmClass: " << startupWmClass;
        }
        
        QString desktopFileName = it.key().split("/").last().replace(".desktop", "");
        qDebug() << "desktopFileName " << desktopFileName << ", it.key():" << it.key().split("/").last().replace(".desktop", "");
        if (startupWmClass.toLower() == appId_.toLower() || desktopFileName.toLower() == appId_.toLower()) {
            //qDebug() << "findDesktopFileByAppId: appid =" << appId_ << it.key();
            PRINT_DBG("findDesktopFileByAppId: appid = %s %s\n", appId_.toStdString().c_str(), it.key().toStdString().c_str());
            return it.key();
        }

        ++it;
    }
    return QString();
}

QString DesktopFileManager::findDesktopFileByRealCmd(QString &cmdline)
{
    QString cmdlineResult = cmdline;
    cmdlineResult = cmdlineResult.remove(QRegExp("\\s+"));
    QString desktopFile;
    auto cmdlineContainsScriptCmd = [cmdlineResult](const DesktopExecMap &desktopRealCmds) {
        auto it = desktopRealCmds.constBegin();
        while (it != desktopRealCmds.constEnd()) {
            auto desktopFile = it.key();
            QString exec;
            QStringList realCmds;
            std::tie(exec, realCmds) = it.value();
            for (auto &cmdItem : realCmds) {
                if (cmdlineResult.contains(cmdItem)) {
                    return desktopFile;
                }
            }
            ++it;
        }
        return QString();
    };

    desktopFile = cmdlineContainsScriptCmd(m_desktopExecRealCmds);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }

    return QString();
}

QString DesktopFileManager::findDesktopFileByTitle(const int &pid, const QString &cmdline)
{
    QString title;
    const auto windows = waylandDisplay()->getWindows();
    for (const auto &window : windows) {
        if (window.second->pid == pid) {
            title = QString::fromStdString(window.second->title);
            break;
        }
    }

    title = title.trimmed();
    QString desktopFile;
    QString cmdlineResult = cmdline;
    cmdlineResult = cmdlineResult.trimmed();
    auto desktopFileEqualName = [title, cmdlineResult](const QMap<QString, QStringList> &namesMap, const DesktopExecMap &desktopsExec) {
        auto it = namesMap.constBegin();
        while (it != namesMap.constEnd()) {
            auto desktopFile = it.key();
            QStringList names = it.value();
            if (names.contains(title)) {
                QString exec;
                QStringList realCmds;
                std::tie(exec, realCmds) = desktopsExec[desktopFile];
                QString execPath = exec.left(exec.lastIndexOf("/"));
                if (cmdlineResult.contains(execPath)) {
                    return desktopFile;
                }
            }
            ++it;
        }
        return QString();
    };

    desktopFile = desktopFileEqualName(m_desktopNames, m_desktopExecs);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }
    return QString();
}

QString DesktopFileManager::findKwreProgramInstallPath(const QString &cmdline)
{
    QString cmdlineBak = cmdline;
    // cmdlineResult = cmdlineResult.remove(QRegExp("\\s+"));
    cmdlineBak = cmdlineBak.trimmed();
    cmdlineBak = cmdlineBak.remove(QRegExp("\\\\"));
    QStringList cmdlineArgs = cmdlineBak.split(" ");
    QString cmdResult;
    for (auto &cmdlineArg : cmdlineArgs) {
        if (cmdlineArg.startsWith("-"))
            break;
        cmdResult.push_back(cmdlineArg);
    }
    auto it = m_kwreCmdLines.constBegin();
    while (it != m_kwreCmdLines.constEnd()) {
        auto installPath = it.key();
        for (const auto &cmdlineItem : it.value()) {
            if (cmdlineItem.contains(cmdResult)) {
                return installPath;
            }
        }
        ++it;
    }
    return QString();
}

QString DesktopFileManager::findKareProgramInstallPath(const QString &cmdline)
{

    auto it = m_kareCmdLines.constBegin();
    QString cmdlineResult = cmdline.trimmed();
    while (it != m_kareCmdLines.constEnd()) {
        auto installPath = it.key();
        for (const auto &cmdlineItem : it.value()) {
            if (cmdlineItem.contains(cmdlineResult) || cmdlineResult.contains(cmdlineItem)) {
                return installPath;
            }
        }
        ++it;
    }
    return QString();
}

QString DesktopFileManager::findKmreAppDesktopFileByCmdline(const QString &cmdline)
{
    // eg. /usr/bin/kylin-kmre-window -i 5001 -w 720 -h 1280 -n 微信 -p com.tencent.mm -f 0 -r 0
    QString kmreAppDesktopFile;
    QRegExp regExp("-p\\s(\\S+)");
    if (regExp.indexIn(cmdline) == -1) {
        PRINT_DBG("regExp.indexIn is error! cmd line %s\n", cmdline.toStdString().c_str());
        return QString();
    }
    kmreAppDesktopFile = regExp.cap(1);
    if (!kmreAppDesktopFile.endsWith(".desktop")) {
        kmreAppDesktopFile.push_back(".desktop");
    }
    return kmreAppDesktopFile;
}

QString DesktopFileManager::findKmreFullDesktopFileByCmdline(const QString &cmdline)
{
    QString kmreAppDesktopFile = findKmreAppDesktopFileByCmdline(cmdline);
    QString kmreAppFullDesktopFile = desktopFilePath(kmreAppDesktopFile);
    if (kmreAppFullDesktopFile.isEmpty()) {
        PRINT_DBG("kmreAppFullDesktopFile is empty! cmdline: %s\n", cmdline.toStdString().c_str());
        return QString();
    }
    return kmreAppFullDesktopFile;
}

QString DesktopFileManager::findDesktopFileByCompareExec(const QString &cmdline, FindDesktopFilePredicate findDesktopFilePredicate)
{
    auto desktopExecIt = std::find_if(m_desktopExecs.constBegin(), m_desktopExecs.constEnd(),
                                      [cmdline, findDesktopFilePredicate](const std::tuple<QString, QStringList> &exec) {
        return findDesktopFilePredicate(exec);
    });
    if (desktopExecIt != m_desktopExecs.constEnd()) {
        return desktopExecIt.key();
    }

    return QString();
}

QString DesktopFileManager::findDesktopFileByEqualExec(const QString &cmdline)
{
    QString desktopFile;
    auto desktopFileEqualExec = [cmdline](const DesktopExecMap &execMap) {
        auto it = execMap.constBegin();
        while (it != execMap.constEnd()) {
            auto desktopFile = it.key();
            QString exec;
            QStringList args;
            std::tie(exec, args) = it.value();
            if (exec == cmdline && args.count() < 2) {
                return desktopFile;
            }
            ++it;
        }
        return QString();
    };

    // DesktopExecMap standardPathDesktopExecMap = standardPathDesktopFileExecMap();
    // desktopFile = desktopFileEqualExec(standardPathDesktopExecMap);
    desktopFile = desktopFileEqualExec(m_desktopExecs);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }

    return QString();
}

QString DesktopFileManager::findDesktopFileByContainsStartArgs(const QString &cmdline)
{
    auto containFullStartArgs = [cmdline](const std::tuple<QString, QStringList> &exec) {
        QString strExec;
        for (const auto &str : std::get<1>(exec)) {
            strExec.push_back(str);
            strExec.push_back(" ");
        }
        if (strExec.endsWith(" ")) {
            strExec.remove(strExec.size() - 1, 1);
        }

        return cmdline.contains(strExec);
    };
    return findDesktopFileByCompareExec(cmdline, containFullStartArgs);
}

QString DesktopFileManager::findDesktopFileByContainsExec(const QString &cmdline)
{
    auto containDesktopFileExec = [cmdline](const std::tuple<QString, QStringList> &exec) {
        // flatpak app
        if (cmdline.contains("bwrap --args")) {
            QString appName = cmdline.split(" ").last();
            const auto args = std::get<1>(exec);
            for (const auto &arg : args) {
                if (arg.contains(appName)) {
                    return true;
                }
            }
            return false;
        }

        // kylin-screenshot sh
        // /usr/bin/vlc --started-from-file /usr/bin/vlc
        if (cmdline.contains("/") && !std::get<0>(exec).contains("/")) {
            const auto cmdlineItems = cmdline.split("/");
            return cmdlineItems.contains(std::get<0>(exec));
        }

        return cmdline == std::get<0>(exec);
    };
    return findDesktopFileByCompareExec(cmdline, containDesktopFileExec);
}

QString DesktopFileManager::findDesktopFileByDpkgCommand(const int &pid)
{
    if (pid <= 0) {
        PRINT_NOTICE("pid is inValid!\n");
        return QString();
    }
    auto future = std::async(std::launch::async, [this, pid]() {
        QString cmdline = QString::fromStdString(process_info_helper::cmdline(pid));
        QString binaryPath = binaryPathFromCmdline(cmdline);
        if (binaryPath.isEmpty()) {
            return QString();
        }
        QString packageName = findPackageNameFromBinaryPath(binaryPath);
        if (packageName.isEmpty()) {
            return QString();
        }
        QString desktopFile = findDesktopFileFromPackageName(packageName, cmdline);
        if (!desktopFile.isEmpty()) {
            return desktopFile;
        }

        return QString();
    });

    std::future_status futureStatus;
    do {
        futureStatus = future.wait_for(std::chrono::milliseconds(1));
        qApp->processEvents();
    } while (futureStatus != std::future_status::ready);
    return future.get();
}

QString DesktopFileManager::binaryPathFromCmdline(const QString &cmdline)
{
    if (cmdline.isEmpty()) {
        PRINT_DBG("cmdline is empty.\n");
        return QString();
    }
    if (QDir::isAbsolutePath(cmdline)) {
        return cmdline;
    }
    QString binaryName = cmdline;
    if (binaryName.startsWith("./")) {
        binaryName.remove("./");
    }

    QString environmentPath = QProcessEnvironment::systemEnvironment().value("PATH");
    QStringList environmentPathLists;
    environmentPathLists << environmentPath.split(':');
    QString cmdlinePath;
    for (auto &path : qAsConst(environmentPathLists)) {
        cmdlinePath = path + "/" + binaryName;
        QFileInfo file(cmdlinePath);
        if (file.isFile()) {
            return cmdlinePath;
        }
    }
    return cmdline;
}

QString DesktopFileManager::findPackageNameFromBinaryPath(const QString &binaryPath)
{
    QProcess findPackageNameProcess;
    QStringList binaryPathList = binaryPath.split(" ");
    if (binaryPathList.isEmpty()) {
        return QString();
    }
    findPackageNameProcess.start("/usr/bin/dpkg", {"-S", binaryPathList.first()});
    findPackageNameProcess.waitForFinished();
    QString packageNameInfo = findPackageNameProcess.readAll();
    QStringList packageInfoList;
    QStringList packNameList;
    packageInfoList << packageNameInfo.split("\n");
    for (auto &packageNameinfo : qAsConst(packageInfoList)) {
        if (!packageNameinfo.isEmpty()) {
            packNameList << packageNameinfo.split(":").first();
        }
    }
    if (packNameList.isEmpty()) {
        return QString();
    }

    return packNameList.first();
}

QString DesktopFileManager::findDesktopFileFromPackageName(const QString &packageName, const QString &cmdline)
{
    if (packageName.isEmpty()) {
        PRINT_DBG("packageName is empty! %s\n", cmdline.toStdString().c_str());
        return QString();
    }
    QStringList desktopFileLists = desktopFileListFromPackageName(packageName);
    if (desktopFileLists.count() == 1) {
        return desktopFileLists.first();
    }
    QString desktopFile = bestDesktopFileFromMultDesktopFileLists(cmdline, desktopFileLists);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }

    return QString();
}

bool DesktopFileManager::desktopFileNoDisplay(const QString &desktopFile)
{
    if (desktopFile.isEmpty()) {
        PRINT_DBG("desktopFileName is NULL!!!\n");
        return false;
    }
    QString content = desktopEntryFromDesktopFile(desktopFile, "NoDisplay");
    if (content.isEmpty()) {
        PRINT_DBG("NoDisplay is empty!\n");
        return false;
    }
    if (content == "true") {
        return true;
    }
    return false;
}

QString DesktopFileManager::desktopEntryFromDesktopFile(const QString &desktopFile, const QString &key)
{
    QString fullDesktopFile = desktopFilePath(desktopFile);
    KDesktopFile desktop(fullDesktopFile);
    QString content = desktop.entryMap("Desktop Entry").value(key);
    if (content.isEmpty()) {
        PRINT_DBG("%s is empty!\n", key.toStdString().c_str());
        return QString();
    }
    content.remove(QRegExp("%."));
    content.remove(QRegExp("^\""));
    content.remove(QRegExp(" *$"));
    content.remove(QRegExp("\""));
    return content;
}

QStringList DesktopFileManager::desktopFileListFromPackageName(const QString &packageName)
{
    QProcess findDesktopFileProcess;
    findDesktopFileProcess.start("/usr/bin/dpkg", {"-L", packageName});
    findDesktopFileProcess.waitForFinished();
    QString desktopFileInfo = findDesktopFileProcess.readAll();
    QStringList desktopFileInfoList;
    QStringList desktopFileList;
    desktopFileInfoList << desktopFileInfo.split("\n");
    for (auto &desktopFileInfo : qAsConst(desktopFileInfoList)) {
        if (desktopFileInfo.isEmpty() || !desktopFileInfo.trimmed().endsWith(".desktop")) {
            continue;
        }
        QStringList desktopDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
        for (auto &desktopPath : desktopDirs) {
            if (!desktopFileInfo.trimmed().contains(desktopPath) || desktopFileNoDisplay(desktopFileInfo.trimmed())) {
                continue;
            }
            desktopFileList << desktopFileInfo.trimmed();
        }
    }

    return desktopFileList;
}

QString DesktopFileManager::bestDesktopFileFromMultDesktopFileLists(const QString &cmdline, const QStringList &desktopFileLists)
{
    QString appCmdline = cmdline;
    if (cmdline.contains("/")) {
        int cmdlineFirst = cmdline.lastIndexOf("/");
        appCmdline = cmdline.right(cmdline.length() - cmdlineFirst - 1);
    }
    // 1. 判断exec与cmdline全等
    for (auto &desktopFile : desktopFileLists) {
        auto exec = getExecByDesktopFile(desktopFile);
        if (std::get<0>(exec) == appCmdline && std::get<1>(exec).count() < 2) {
            return desktopFile;
        }
    }
    // 2. 判断启动路径+参数与cmdline全等
    for (auto &desktopFile : desktopFileLists) {
        auto exec = getExecByDesktopFile(desktopFile);
        QString startArgs = std::get<1>(exec).join(" ");
        if (startArgs == appCmdline) {
            return desktopFile;
        }
    }
    // 3. 判断cmdline包含启动路径+参数
    for (auto &desktopFile : desktopFileLists) {
        auto exec = getExecByDesktopFile(desktopFile);
        QString startArgs = std::get<1>(exec).join(" ");
        if (appCmdline.contains(startArgs)) {
            return desktopFile;
        }
    }

    return QString();
}

QString DesktopFileManager::findDesktopFileByEqualStartArg(const QString &cmdline)
{
    QString desktopFile;
    auto desktopFileEqualStartArg = [cmdline](const DesktopExecMap &execMap) {
        auto it = execMap.constBegin();
        while (it != execMap.constEnd()) {
            QString exec;
            QStringList args;
            std::tie(exec, args) = it.value();
            QString startArgs = args.join(" ");
            if (startArgs == cmdline) {
                return it.key();
            }
            ++it;
        }
        return QString();
    };

    desktopFile = desktopFileEqualStartArg(m_desktopExecs);
    if (!desktopFile.isEmpty()) {
        return desktopFile;
    }

    return QString();
}

QString DesktopFileManager::findDesktopFileFromPid(int pid)
{
    if (pid <= 0) {
        PRINT_DBG("pid is invalid!\n");
        return QString();
    }
    std::string environFile = "/proc/" + std::to_string(pid) + "/environ";
    const QString gioDesktopFile =
        readDesktopFileFromEnviron(environFile, "GIO_LAUNCHED_DESKTOP_FILE=");
    if (gioDesktopFile.isEmpty()) {
        return readDesktopFileFromEnviron(environFile, "PANSHI_APP_DESKTOP=");
    }

    return gioDesktopFile;
}

QString DesktopFileManager::readDesktopFileFromEnviron(
    const std::string &environFile, const QString &environName)
{
    std::ifstream processEnvironFile(environFile);
    if (!processEnvironFile.is_open()) {
        return QString();
    }
    std::string line;
    while (std::getline(processEnvironFile, line, '\0')) {
        auto pos = line.find(environName.toStdString());
        if (pos != std::string::npos) {
            line.erase(pos, environName.length());
            return QString::fromStdString(line);
        }
    }
    return QString();
}

DesktopFileManager::DesktopExecMap DesktopFileManager::standardPathDesktopFileExecMap()
{
    DesktopExecMap standardPathDesktopExecMap;
    QStringList standardPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    auto it = m_desktopExecs.constBegin();
    while (it != m_desktopExecs.constEnd()) {
        auto desktopFile = it.key();
        for (const auto &path : standardPaths) {
            if (desktopFile.contains(path)) {
                standardPathDesktopExecMap[desktopFile] = it.value();
            }
        }
        ++it;
    }

    return standardPathDesktopExecMap;
}

DesktopFileManager::DesktopExecMap DesktopFileManager::autoStartPathDesktopFileExecMap()
{
    DesktopExecMap autoStartPathDesktopExecMap;
    auto it = m_desktopExecs.constBegin();
    while (it != m_desktopExecs.constEnd()) {
        auto desktopFile = it.key();
        if (desktopFile.contains(XDG_AUTO_START_PATH)) {
            autoStartPathDesktopExecMap[desktopFile] = it.value();
        }
        ++it;
    }
    return autoStartPathDesktopExecMap;
}

bool DesktopFileManager::isKylinOsManagerTool(int pid)
{
    QString cmdline = QString::fromStdString(process_info_helper::cmdline(pid));
    return cmdline.startsWith("/usr/lib/kylin-os-manager/bin");
}
