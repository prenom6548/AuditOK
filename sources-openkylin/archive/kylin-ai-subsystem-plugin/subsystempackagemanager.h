/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SUBSYSTEMPACKAGEMANAGER_H
#define SUBSYSTEMPACKAGEMANAGER_H

#include <QObject>
#include <QStringList>

class SubSystemPackageManager : public QObject {
    Q_OBJECT
public:
    explicit SubSystemPackageManager(QObject *parent = nullptr);

    void installPackage();
    void unInstallPackage(const QStringList &uninstallList);
    void reboot();
    void fixBrokenDepends();
    void fixIncompleteInstall();

private:
    void connectSignals();
    void updateCache();

private slots:
    void handleUpdateCacheFinished(bool success, const QStringList &groupList,
                                   const QString &errorResult,
                                   const QString &errorReason);
    void handleUpdateCacheStatusChanged(int progress, const QString &status);

    void handleInstalldebStatusChanged(int progress, const QString &status,
                                       const QString &details);
    void handleUpdateInstallFinished(bool success, const QStringList &groupList,
                                     const QString &errorResult,
                                     const QString &errorReason);

    void handlePurgePackagesStatusChanged(int progress, const QString &status,
                                          const QString &details);

    void handlePurgePackagesFinished(bool success, const QString &status,
                                     const QString &details,
                                     const QString &pkgNameListString);

signals:
    void installProgressChanged(int progress);
    void installSuccessed();
    void installFailed(QString errorResult, QString errorReason);
    void uninstallProgressChanged(int progress);
    void unInstallSuccessed();
    void unIntsallFailed();

private:
    const QString serviceName_ = "com.kylin.systemupgrade";
    const QString objectPath_ = "/com/kylin/systemupgrade";
    const QString interfaceName_ = "com.kylin.systemupgrade.interface";

    const QStringList installPackageNames_ = {"kylin-ai-subsystem"};
    const QStringList uninstallPackageNames_ = {
        "kylin-ai-subsystem",           "kylin-ai-runtime",
        "kylin-ai-engine-plugins",      "kylin-ai-subsystem-modelconfig",
        "kyai-data-management-service", "kylin-ai-document-qa-service",
        "kylin-ai-document-service",    "kylin-ai-vector-engine",
        "kylin-ai-python-env",          "libtriton-server"};
    const QStringList installList_ = {
        "libkylin-ondevice-traditional-ai-engine-plugin",
        "libkyai-business-framework",
        "kylin-ai-document-qa-service",
        "libkylin-ai-document-qa-service",
        "kylin-ai-document-service",
        "libkylin-ai-document-service",
        "kylin-ai-python-env",
        "libkysdk-vector-engine-client",
        "kylin-ai-vector-engine",
        "kylin-ai-runtime",
        "libkylin-coreai-embedding",
        "libkyai-assistant0",
        "libkyai-config0",
        "libkysdk-coreai-speech0",
        "libkysdk-coreai-vision0",
        "libkysdk-genai-nlp0",
        "libkysdk-genai-vision0",
        "kylin-ai-subsystem-modelconfig"};

    bool isUpdatingCache_ = false;
    bool isInstallingPkg_ = false;
    bool isUninstallingPkg = false;
};

#endif // SUBSYSTEMPACKAGEMANAGER_H
