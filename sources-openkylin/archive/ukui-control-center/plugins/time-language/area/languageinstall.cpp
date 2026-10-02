/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "languageinstall.h"

#include "languagefeatureconfig.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDebug>
#include <QNetworkInterface>
#include <QUuid>
#include <QVariant>

#include <kysdk/kysdk-system/libkypackages.h>

namespace {
const char kSystemDbusService[] = "com.control.center.qt.systemdbus";
const char kSystemDbusPath[] = "/";
const char kSystemDbusInterface[] = "com.control.center.interface";
const int kPackageInstallCallTimeoutMs = 120000;

// 获取默认安装失败文案
QString defaultInstallErrorText()
{
    return QCoreApplication::translate("Area", "Installation failed. Please retry later.");
}

// 获取网络不可用提示文案
QString networkUnavailableText()
{
    return QCoreApplication::translate("LanguageFeatureItem",
                                       "Unable to connect to the network. Please reconnect and retry installation.");
}

// 获取默认卸载失败文案
QString defaultUninstallErrorText()
{
    return QCoreApplication::translate("Area", "Uninstallation failed. Please retry later.");
}

// 添加去重后的字符串
void appendUniqueString(QStringList *list, const QString &value)
{
    if (!list) {
        return;
    }
    const QString normalizedValue = value.trimmed();
    if (!normalizedValue.isEmpty() && !list->contains(normalizedValue)) {
        list->append(normalizedValue);
    }
}

// 通过 SDK 查询软件包安装状态
bool isPackageInstalledWithSdk(const QString &packageName)
{
    const QString normalizedPackageName = packageName.trimmed();
    if (normalizedPackageName.isEmpty()) {
        return true;
    }

    const int versionSeparator = normalizedPackageName.indexOf(QLatin1Char('='));
    const QString queryPackageName = versionSeparator > 0
            ? normalizedPackageName.left(versionSeparator)
            : normalizedPackageName;
    const QString expectedVersion = versionSeparator > 0
            ? normalizedPackageName.mid(versionSeparator + 1)
            : QString();

    const QByteArray packageNameBytes = queryPackageName.toUtf8();
    const QByteArray packageVersionBytes = expectedVersion.toUtf8();
    return kdk_package_is_installed(packageNameBytes.constData(),
                                    expectedVersion.isEmpty()
                                        ? nullptr
                                        : packageVersionBytes.constData()) == 0;
}

// 查询 NetworkManager 是否联网
bool isNetworkManagerAvailable(bool *checked)
{
    if (checked) {
        *checked = false;
    }

    QDBusInterface properties(QStringLiteral("org.freedesktop.NetworkManager"),
                              QStringLiteral("/org/freedesktop/NetworkManager"),
                              QStringLiteral("org.freedesktop.DBus.Properties"),
                              QDBusConnection::systemBus());
    if (!properties.isValid()) {
        return false;
    }

    if (checked) {
        *checked = true;
    }

    QDBusReply<QVariant> connectivity = properties.call(QStringLiteral("Get"),
                                                        QStringLiteral("org.freedesktop.NetworkManager"),
                                                        QStringLiteral("Connectivity"));
    if (connectivity.isValid()) {
        const uint value = connectivity.value().toUInt();
        if (value != 0) {
            return value == 4;
        }
    }

    QDBusReply<QVariant> state = properties.call(QStringLiteral("Get"),
                                                 QStringLiteral("org.freedesktop.NetworkManager"),
                                                 QStringLiteral("State"));
    if (state.isValid()) {
        return state.value().toUInt() >= 50;
    }

    return false;
}
} // namespace

// 构造语言特性安装管理器
LanguageInstall::LanguageInstall(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<PackageInstallResult>("PackageInstallResult");
    qRegisterMetaType<LanguageFeatureInstallTask>("LanguageFeatureInstallTask");
    QDBusConnection::systemBus().connect(kSystemDbusService,
                                         kSystemDbusPath,
                                         kSystemDbusInterface,
                                         QStringLiteral("packageInstallProgress"),
                                         this,
                                         SLOT(onPackageInstallProgress(QString,QString,int,int,QString)));
    QDBusConnection::systemBus().connect(kSystemDbusService,
                                         kSystemDbusPath,
                                         kSystemDbusInterface,
                                         QStringLiteral("packageInstallFinished"),
                                         this,
                                         SLOT(onPackageInstallFinished(QString,QString,bool,QString)));
    QDBusConnection::systemBus().connect(kSystemDbusService,
                                         kSystemDbusPath,
                                         kSystemDbusInterface,
                                         QStringLiteral("packageUninstallProgress"),
                                         this,
                                         SLOT(onPackageUninstallProgress(QString,QString,int,int,QString)));
    QDBusConnection::systemBus().connect(kSystemDbusService,
                                         kSystemDbusPath,
                                         kSystemDbusInterface,
                                         QStringLiteral("packageUninstallFinished"),
                                         this,
                                         SLOT(onPackageUninstallFinished(QString,QString,bool,QString)));
}

// 析构语言特性安装管理器
LanguageInstall::~LanguageInstall()
{
    QDBusConnection::systemBus().disconnect(kSystemDbusService,
                                            kSystemDbusPath,
                                            kSystemDbusInterface,
                                            QStringLiteral("packageInstallProgress"),
                                            this,
                                            SLOT(onPackageInstallProgress(QString,QString,int,int,QString)));
    QDBusConnection::systemBus().disconnect(kSystemDbusService,
                                            kSystemDbusPath,
                                            kSystemDbusInterface,
                                            QStringLiteral("packageInstallFinished"),
                                            this,
                                            SLOT(onPackageInstallFinished(QString,QString,bool,QString)));
    QDBusConnection::systemBus().disconnect(kSystemDbusService,
                                            kSystemDbusPath,
                                            kSystemDbusInterface,
                                            QStringLiteral("packageUninstallProgress"),
                                            this,
                                            SLOT(onPackageUninstallProgress(QString,QString,int,int,QString)));
    QDBusConnection::systemBus().disconnect(kSystemDbusService,
                                            kSystemDbusPath,
                                            kSystemDbusInterface,
                                            QStringLiteral("packageUninstallFinished"),
                                            this,
                                            SLOT(onPackageUninstallFinished(QString,QString,bool,QString)));
}

// 查询软件包是否已安装
bool LanguageInstall::isPackageInstalled(const QString &packageName)
{
    const QString normalizedPackageName = packageName.trimmed();
    if (normalizedPackageName.isEmpty()) {
        return true;
    }

    return isPackageInstalledWithSdk(normalizedPackageName);
}

// 查询网络是否可用
bool LanguageInstall::isNetworkAvailable()
{
    bool networkManagerChecked = false;
    const bool networkManagerAvailable = isNetworkManagerAvailable(&networkManagerChecked);
    if (networkManagerChecked) {
        return networkManagerAvailable;
    }

    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp)
                && iface.flags().testFlag(QNetworkInterface::IsRunning)
                && !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            return true;
        }
    }
    return false;
}

// 清空安装和卸载队列
void LanguageInstall::clear()
{
    m_installQueue.clear();
    m_removeQueue.clear();
    if (m_installRunning) {
        for (auto it = m_activeInstallTasks.begin(); it != m_activeInstallTasks.end(); ++it) {
            QList<LanguageFeatureInstallTask> &tasks = it.value();
            for (LanguageFeatureInstallTask &task : tasks) {
                task.item.clear();
            }
        }
    } else {
        m_activeInstallTasks.clear();
        m_activeRequestId.clear();
    }
    if (!m_removeRunning) {
        m_activeRemoveRequestId.clear();
        m_activeRemovePackageNames.clear();
    }
}

// 移除指定语言的待安装任务
void LanguageInstall::removeInstalls(const QString &languageCode)
{
    QQueue<LanguageFeatureInstallJob> remainingQueue;
    while (!m_installQueue.isEmpty()) {
        LanguageFeatureInstallJob job = m_installQueue.dequeue();
        QList<LanguageFeatureInstallTask> remainingTasks;
        for (const LanguageFeatureInstallTask &task : job.tasks) {
            if (task.languageCode != languageCode) {
                remainingTasks.append(task);
            }
        }
        if (!remainingTasks.isEmpty()) {
            job.tasks = remainingTasks;
            remainingQueue.enqueue(job);
        }
    }
    m_installQueue = remainingQueue;

    if (m_installRunning) {
        for (auto it = m_activeInstallTasks.begin(); it != m_activeInstallTasks.end(); ++it) {
            QList<LanguageFeatureInstallTask> &tasks = it.value();
            for (LanguageFeatureInstallTask &task : tasks) {
                if (task.languageCode == languageCode) {
                    task.item.clear();
                }
            }
        }
    }
}

// 加入待卸载软件包队列
void LanguageInstall::removePackages(const QStringList &packageNames)
{
    QStringList packagesToRemove;
    for (const QString &packageName : packageNames) {
        const QString normalizedPackageName = packageName.trimmed();
        if (normalizedPackageName.isEmpty()) {
            continue;
        }
        bool queued = m_activeRemovePackageNames.contains(normalizedPackageName);
        for (const QStringList &queuedPackages : m_removeQueue) {
            if (queuedPackages.contains(normalizedPackageName)) {
                queued = true;
                break;
            }
        }
        if (queued) {
            continue;
        }
        appendUniqueString(&packagesToRemove, normalizedPackageName);
    }
    if (!packagesToRemove.isEmpty()) {
        m_removeQueue.enqueue(packagesToRemove);
    }
    processNextRemove();
}

// 启动单项语言特性安装
void LanguageInstall::startInstall(const QString &languageCode,
                                   LanguageFeatureType type,
                                   LanguageFeatureItem *item,
                                   const InstallLanguageResult &result)
{
    if (!item) {
        return;
    }

    LanguageFeatureInstallTask task;
    task.languageCode = languageCode;
    task.type = type;
    task.item = item;
    task.result = result;
    task.packageName = packageNameForFeature(result, type);

    if (updateExistingInstallTask(task)) {
        return;
    }

    item->setState(LanguageFeatureState::Installing, 0);
    LanguageFeatureInstallJob job;
    job.tasks.append(task);
    m_installQueue.enqueue(job);
    processNextInstall();
}

// 启动批量语言特性安装
void LanguageInstall::startInstallBatch(
        const QString &languageCode,
        const QList<QPair<LanguageFeatureType, LanguageFeatureItem *>> &features,
        const InstallLanguageResult &result)
{
    LanguageFeatureInstallJob job;
    for (const QPair<LanguageFeatureType, LanguageFeatureItem *> &feature : features) {
        if (!feature.second) {
            continue;
        }

        LanguageFeatureInstallTask task;
        task.languageCode = languageCode;
        task.type = feature.first;
        task.item = feature.second;
        task.result = result;
        task.packageName = packageNameForFeature(result, feature.first);

        if (updateExistingInstallTask(task)) {
            continue;
        }

        feature.second->setState(LanguageFeatureState::Installing, 0);
        job.tasks.append(task);
    }

    if (job.tasks.isEmpty()) {
        return;
    }

    m_installQueue.enqueue(job);
    processNextInstall();
}

// 获取语言特性对应的软件包名
QString LanguageInstall::packageNameForFeature(const InstallLanguageResult &result,
                                               LanguageFeatureType type) const
{
    const LanguageFeatureOption option = LanguageFeatureConfig::optionForLanguage(result.languageCode);
    if (type == LanguageFeatureType::Font) {
        return result.fontPackageName.isEmpty()
                ? option.fontPackageName.trimmed()
                : result.fontPackageName.trimmed();
    }

    return result.inputMethodPackageName.isEmpty()
            ? option.inputMethodPackageName.trimmed()
            : result.inputMethodPackageName.trimmed();
}

// 更新已存在的安装任务
bool LanguageInstall::updateExistingInstallTask(const LanguageFeatureInstallTask &task)
{
    if (!task.item) {
        return false;
    }

    if (m_installRunning) {
        for (auto it = m_activeInstallTasks.begin(); it != m_activeInstallTasks.end(); ++it) {
            QList<LanguageFeatureInstallTask> &tasks = it.value();
            for (LanguageFeatureInstallTask &activeTask : tasks) {
                if (activeTask.languageCode == task.languageCode
                        && activeTask.type == task.type) {
                    if (!activeTask.item) {
                        activeTask.item = task.item;
                    }
                    task.item->setState(LanguageFeatureState::Installing, 0);
                    return true;
                }
            }
        }
    }

    for (int i = 0; i < m_installQueue.size(); ++i) {
        LanguageFeatureInstallJob &job = m_installQueue[i];
        for (LanguageFeatureInstallTask &queuedTask : job.tasks) {
            if (queuedTask.languageCode == task.languageCode
                    && queuedTask.type == task.type) {
                queuedTask.item = task.item;
                queuedTask.result = task.result;
                queuedTask.packageName = task.packageName;
                task.item->setState(LanguageFeatureState::Installing, 0);
                return true;
            }
        }
    }

    return false;
}

// 处理下一个安装任务
void LanguageInstall::processNextInstall()
{
    if (m_installRunning || m_removeRunning || m_installQueue.isEmpty()) {
        return;
    }

    LanguageFeatureInstallJob activeJob;
    while (!m_installQueue.isEmpty()) {
        activeJob = m_installQueue.dequeue();
        QList<LanguageFeatureInstallTask> validTasks;
        for (const LanguageFeatureInstallTask &task : activeJob.tasks) {
            if (task.item) {
                validTasks.append(task);
            }
        }
        activeJob.tasks = validTasks;
        if (!activeJob.tasks.isEmpty()) {
            break;
        }
    }
    if (activeJob.tasks.isEmpty()) {
        return;
    }

    m_installRunning = true;
    QStringList packagesToInstall;
    bool networkChecked = false;
    bool networkAvailable = false;

    for (LanguageFeatureInstallTask task : activeJob.tasks) {
        if (task.item) {
            task.item->setState(LanguageFeatureState::Installing, 0);
        }

        PackageInstallResult packageResult;
        packageResult.packageName = task.packageName.trimmed();
        if (packageResult.packageName.isEmpty() || isPackageInstalled(packageResult.packageName)) {
            packageResult.success = true;
            Q_EMIT installFinished(task, packageResult);
            continue;
        }

        if (!networkChecked) {
            networkAvailable = isNetworkAvailable();
            networkChecked = true;
        }
        if (!networkAvailable) {
            qWarning() << Q_FUNC_INFO << "network is unavailable, skip package install:"
                       << packageResult.packageName;
            packageResult.errorText = networkUnavailableText();
            Q_EMIT installFinished(task, packageResult);
            continue;
        }

        appendUniqueString(&packagesToInstall, packageResult.packageName);
        m_activeInstallTasks[packageResult.packageName].append(task);
    }

    if (m_activeInstallTasks.isEmpty()) {
        finishInstallJob();
        return;
    }

    QDBusInterface interface(kSystemDbusService,
                             kSystemDbusPath,
                             kSystemDbusInterface,
                             QDBusConnection::systemBus());
    if (!interface.isValid()) {
        qWarning() << Q_FUNC_INFO << "system dbus interface is invalid:"
                   << QDBusConnection::systemBus().lastError();
        failActiveInstallPackages(defaultInstallErrorText());
        return;
    }

    m_activeRequestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    qInfo() << Q_FUNC_INFO << "request package install from system dbus:"
            << packagesToInstall << "request:" << m_activeRequestId;

    const QString requestId = m_activeRequestId;
    QDBusMessage message = QDBusMessage::createMethodCall(kSystemDbusService,
                                                          kSystemDbusPath,
                                                          kSystemDbusInterface,
                                                          QStringLiteral("installPackagesOnline"));
    message << packagesToInstall << requestId;
    QDBusPendingCall pendingCall = QDBusConnection::systemBus().asyncCall(message,
                                                                         kPackageInstallCallTimeoutMs);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *call) {
        QDBusPendingReply<bool> reply = *call;
        call->deleteLater();
        if (!m_installRunning || m_activeRequestId != requestId) {
            return;
        }
        if (reply.isError() || !reply.value()) {
            qWarning() << Q_FUNC_INFO << "failed to start package install:"
                       << packagesToInstall << "request:" << requestId
                       << "error:" << reply.error();
            failActiveInstallPackages(defaultInstallErrorText());
        }
    });
}

// 完成指定软件包的安装任务
void LanguageInstall::finishActiveInstallPackage(const QString &packageName,
                                                 const PackageInstallResult &packageResult)
{
    const QList<LanguageFeatureInstallTask> tasks = m_activeInstallTasks.take(packageName.trimmed());
    for (const LanguageFeatureInstallTask &task : tasks) {
        Q_EMIT installFinished(task, packageResult);
    }

    if (m_activeInstallTasks.isEmpty()) {
        finishInstallJob();
    }
}

// 将当前安装任务全部置为失败
void LanguageInstall::failActiveInstallPackages(const QString &errorText)
{
    const QHash<QString, QList<LanguageFeatureInstallTask>> activeTasks = m_activeInstallTasks;
    m_activeInstallTasks.clear();

    for (auto it = activeTasks.constBegin(); it != activeTasks.constEnd(); ++it) {
        PackageInstallResult failedResult;
        failedResult.packageName = it.key();
        failedResult.errorText = errorText;
        for (const LanguageFeatureInstallTask &task : it.value()) {
            Q_EMIT installFinished(task, failedResult);
        }
    }

    finishInstallJob();
}

// 结束当前安装任务
void LanguageInstall::finishInstallJob()
{
    m_activeInstallTasks.clear();
    m_activeRequestId.clear();
    m_installRunning = false;

    processNextRemove();
    processNextInstall();
}

// 处理下一个卸载任务
void LanguageInstall::processNextRemove()
{
    if (m_removeRunning || m_installRunning || m_removeQueue.isEmpty()) {
        return;
    }

    while (!m_removeQueue.isEmpty()) {
        m_activeRemovePackageNames = m_removeQueue.dequeue();
        QStringList normalizedPackageNames;
        for (const QString &packageName : m_activeRemovePackageNames) {
            appendUniqueString(&normalizedPackageNames, packageName);
        }
        m_activeRemovePackageNames = normalizedPackageNames;
        if (!m_activeRemovePackageNames.isEmpty()) {
            break;
        }
    }
    if (m_activeRemovePackageNames.isEmpty()) {
        return;
    }

    QDBusInterface interface(kSystemDbusService,
                             kSystemDbusPath,
                             kSystemDbusInterface,
                             QDBusConnection::systemBus());
    if (!interface.isValid()) {
        qWarning() << Q_FUNC_INFO << "system dbus interface is invalid:"
                   << QDBusConnection::systemBus().lastError();
        const QStringList packageNames = m_activeRemovePackageNames;
        for (const QString &packageName : packageNames) {
            PackageInstallResult failedResult;
            failedResult.packageName = packageName;
            failedResult.errorText = defaultUninstallErrorText();
            finishRemovePackage(failedResult);
        }
        finishRemoveJob();
        return;
    }

    m_removeRunning = true;
    m_activeRemoveRequestId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString requestId = m_activeRemoveRequestId;
    const QStringList packageNames = m_activeRemovePackageNames;
    qInfo() << Q_FUNC_INFO << "request package uninstall from system dbus:"
            << packageNames << "request:" << requestId;

    QDBusMessage message = QDBusMessage::createMethodCall(kSystemDbusService,
                                                          kSystemDbusPath,
                                                          kSystemDbusInterface,
                                                          QStringLiteral("uninstallPackages"));
    message << packageNames << requestId;
    QDBusPendingCall pendingCall = QDBusConnection::systemBus().asyncCall(message,
                                                                         kPackageInstallCallTimeoutMs);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *call) {
        QDBusPendingReply<bool> reply = *call;
        call->deleteLater();
        if (!m_removeRunning || m_activeRemoveRequestId != requestId) {
            return;
        }
        if (reply.isError() || !reply.value()) {
            qWarning() << Q_FUNC_INFO << "failed to start package uninstall:"
                       << packageNames << "request:" << requestId
                       << "error:" << reply.error();
            for (const QString &packageName : packageNames) {
                PackageInstallResult failedResult;
                failedResult.packageName = packageName;
                failedResult.errorText = defaultUninstallErrorText();
                finishRemovePackage(failedResult);
            }
            finishRemoveJob();
        }
    });
}

// 完成指定软件包的卸载任务
void LanguageInstall::finishRemovePackage(const PackageInstallResult &packageResult)
{
    if (!packageResult.success) {
        qWarning() << Q_FUNC_INFO << "package uninstall failed:"
                   << packageResult.packageName << packageResult.errorText;
    }

    m_activeRemovePackageNames.removeAll(packageResult.packageName.trimmed());
}

// 结束当前卸载任务
void LanguageInstall::finishRemoveJob()
{
    m_activeRemoveRequestId.clear();
    m_activeRemovePackageNames.clear();
    m_removeRunning = false;

    processNextRemove();
    processNextInstall();
}

// 更新软件包安装进度
void LanguageInstall::onPackageInstallProgress(const QString &requestId,
                                               const QString &packageName,
                                               int progress,
                                               int state,
                                               const QString &logInfo)
{
    Q_UNUSED(state);
    Q_UNUSED(logInfo);

    if (!m_installRunning
            || m_activeRequestId != requestId
            || !m_activeInstallTasks.contains(packageName)) {
        return;
    }

    const int normalizedProgress = qBound(0, progress, 100);
    const QList<LanguageFeatureInstallTask> tasks = m_activeInstallTasks.value(packageName);
    for (const LanguageFeatureInstallTask &task : tasks) {
        if (task.item) {
            task.item->setState(LanguageFeatureState::Installing, normalizedProgress);
        }
    }
}

// 处理软件包安装完成信号
void LanguageInstall::onPackageInstallFinished(const QString &requestId,
                                               const QString &packageName,
                                               bool success,
                                               const QString &errorText)
{
    if (!m_installRunning
            || m_activeRequestId != requestId
            || !m_activeInstallTasks.contains(packageName)) {
        return;
    }

    PackageInstallResult result;
    result.packageName = packageName;
    result.success = success;
    result.errorText = success
            ? QString()
            : (errorText.isEmpty() ? defaultInstallErrorText() : errorText);
    finishActiveInstallPackage(packageName, result);
}

// 接收软件包卸载进度信号
void LanguageInstall::onPackageUninstallProgress(const QString &requestId,
                                                 const QString &packageName,
                                                 int progress,
                                                 int state,
                                                 const QString &logInfo)
{
    Q_UNUSED(progress);
    Q_UNUSED(state);
    Q_UNUSED(logInfo);

    if (!m_removeRunning
            || m_activeRemoveRequestId != requestId
            || !m_activeRemovePackageNames.contains(packageName)) {
        return;
    }
}

// 处理软件包卸载完成信号
void LanguageInstall::onPackageUninstallFinished(const QString &requestId,
                                                 const QString &packageName,
                                                 bool success,
                                                 const QString &errorText)
{
    if (!m_removeRunning
            || m_activeRemoveRequestId != requestId
            || !m_activeRemovePackageNames.contains(packageName)) {
        return;
    }

    PackageInstallResult result;
    result.packageName = packageName;
    result.success = success;
    result.errorText = success
            ? QString()
            : (errorText.isEmpty() ? defaultUninstallErrorText() : errorText);
    finishRemovePackage(result);
    if (m_activeRemovePackageNames.isEmpty()) {
        finishRemoveJob();
    }
}
