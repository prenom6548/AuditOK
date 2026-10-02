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

#ifndef DESKTOPFILEMANAGER_H
#define DESKTOPFILEMANAGER_H

#include <QFuture>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <tuple>
class DesktopFileManager : public QObject
{
    Q_OBJECT
public:
    static DesktopFileManager &instance()
    {
        static DesktopFileManager instance;
        return instance;
    }

    std::string syncGetDesktopFileByPid(int pid, bool needUpDateDesktopTable = true);
    std::string syncGetDesktopFileByAppId(const std::string &appId);
    std::vector<std::string> desktopFilesWithName(const std::string &desktopName);
    std::string findDesktopFileByTitle1(const std::string &wid, const int &pid);
    // todo: async

private:
    explicit DesktopFileManager(QObject *parent = nullptr);

    void initAllDirDesktopSize();

    bool hasDirDesktopfileSizeChanged();

    int getFileCount(const QString &path);

    using FindDesktopFilePredicate = std::function<bool(std::tuple<QString, QStringList>)>;
    using DesktopExecMap = QMap<QString, std::tuple<QString, QStringList>>;
    // using CmdLineMap = QMap<QString, QStringList>;

    void loadDesktopFiles();
    void loadKwreCmdLineFiles();
    void loadKareCmdLineFiles();

    void loadDesktopFilesSync();
    void loadKwreCmdLineFilesSync();
    void loadKareCmdLineFilesSync();

    QStringList desktopFilePaths() const;
    QStringList getCmdLineByProgramFile(const QString &programFilePath, const QString &type);
    void updateKwreCmdlinesInfo(const QString &subPath, const QString &path);
    void updateKareCmdlinesInfo(const QString &subPath, const QString &path);
    void updateDesktopExecsInfo(const QString &path);
    void updateDesktopNamesInfo(const QString &path);

    void updateDesktopStartupWmClassInfo(const QString &path);

    // exec, args
    std::tuple<QString, QStringList> getExecByDesktopFile(const QString &desktopFile) const;
    QStringList getAllNamesByDesktopFile(const QString &desktopFile) const;
    QString getStartupWmClass(const QString &desktopFile) const;
    QStringList getRealCmdByExec(const QString &exec) const;
    QStringList getKareRealCmd(const QString &exec) const;
    QString desktopFilePath(const QString &desktopFileName) const;
    QString findDesktopFileByAppId(const QString &appId) const;
    QString findDesktopFileFromLocalCache(const int &pid);
    QString findDesktopFileByRealCmd(QString &cmdline);
    QString findDesktopFileByTitle(const int &pid, const QString &cmdline);
    QString findKwreProgramInstallPath(const QString &cmdline);
    QString findKareProgramInstallPath(const QString &cmdline);
    QString findKmreAppDesktopFileByCmdline(const QString &cmdline);
    QString findKmreFullDesktopFileByCmdline(const QString &cmdline);
    QString findDesktopFileByCompareExec(const QString &cmdline, FindDesktopFilePredicate findDesktopFilePredicate);
    QString findDesktopFileByEqualExec(const QString &cmdline);
    QString findDesktopFileByContainsStartArgs(const QString &cmdline);
    QString findDesktopFileByContainsExec(const QString &cmdline);
    QString findDesktopFileByDpkgCommand(const int &pid);
    QString binaryPathFromCmdline(const QString &cmdline);
    QString findPackageNameFromBinaryPath(const QString &binaryPath);
    QString findDesktopFileFromPackageName(const QString &packageName, const QString &cmdline);
    bool desktopFileNoDisplay(const QString &desktopFile);
    QString desktopEntryFromDesktopFile(const QString &desktopFile, const QString &key);
    QStringList desktopFileListFromPackageName(const QString &packageName);
    QString bestDesktopFileFromMultDesktopFileLists(const QString &cmdline, const QStringList &desktopFileLists);
    QString findDesktopFileByEqualStartArg(const QString &cmdline);
    QString findDesktopFileFromPid(int pid);
    QString readDesktopFileFromEnviron(const std::string &environFile, const QString &environName);
    DesktopExecMap standardPathDesktopFileExecMap();
    DesktopExecMap autoStartPathDesktopFileExecMap();
    bool isKylinOsManagerTool(int pid);

private:
    QFuture<void> m_loadDesktopFilesFuture;
    QFuture<void> m_loadKwreCmdLineFilesFuture;
    QFuture<void> m_loadKareCmdLineFilesFuture;
    QMutex m_mutex;
    // desktopfile, exec, args
    DesktopExecMap m_desktopExecs;
    QMap<QString, QStringList> m_desktopNames;
    QMap<QString, QString> m_desktopStartupWmClass;
    DesktopExecMap m_desktopExecRealCmds;
    QMap<QString, int> m_desktopPathFileCnt;
    QMap<QString, QStringList> m_kwreCmdLines;
    QMap<QString, QStringList> m_kareCmdLines;

    const QString kmreAppMainWindow = "/usr/bin/kylin-kmre-window";
    const QString polkitWindowPidCmdline = "ukui-polkit/polkit-ukui-authentication-agent-1";

    // 保存应用appid到desktopfile的映射关系, 兼容一部分appid与desktopfile不一致的情况
    const QMap<QString, QString> m_AppIdToDesktopFile = {
        {"sqaxsafeforcnos_5", "startqax"},
    };
};

#endif // DESKTOPFILEMANAGER_H
