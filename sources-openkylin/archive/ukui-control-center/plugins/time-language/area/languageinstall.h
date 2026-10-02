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
#ifndef LANGUAGEINSTALL_H
#define LANGUAGEINSTALL_H

#include "installlanguagefeaturesdialog.h"
#include "languagefeatureitem.h"

#include <QObject>
#include <QHash>
#include <QList>
#include <QPair>
#include <QPointer>
#include <QQueue>
#include <QStringList>

struct PackageInstallResult
{
    QString packageName;
    QString errorText;
    bool success = false;
};

struct LanguageFeatureInstallTask
{
    QString languageCode;
    LanguageFeatureType type = LanguageFeatureType::Font;
    QPointer<LanguageFeatureItem> item;
    InstallLanguageResult result;
    QString packageName;
};

struct LanguageFeatureInstallJob
{
    QList<LanguageFeatureInstallTask> tasks;
};

Q_DECLARE_METATYPE(PackageInstallResult)
Q_DECLARE_METATYPE(LanguageFeatureInstallTask)

class LanguageInstall : public QObject
{
    Q_OBJECT

public:
    // 构造语言特性安装管理器
    explicit LanguageInstall(QObject *parent = nullptr);
    // 析构语言特性安装管理器
    ~LanguageInstall() override;

    // 查询软件包是否已安装
    static bool isPackageInstalled(const QString &packageName);
    // 查询网络是否可用
    static bool isNetworkAvailable();

    // 清空安装和卸载队列
    void clear();
    // 移除指定语言的待安装任务
    void removeInstalls(const QString &languageCode);
    // 加入待卸载软件包队列
    void removePackages(const QStringList &packageNames);
    // 启动单项语言特性安装
    void startInstall(const QString &languageCode,
                      LanguageFeatureType type,
                      LanguageFeatureItem *item,
                      const InstallLanguageResult &result);
    // 启动批量语言特性安装
    void startInstallBatch(const QString &languageCode,
                           const QList<QPair<LanguageFeatureType, LanguageFeatureItem *>> &features,
                           const InstallLanguageResult &result);

Q_SIGNALS:
    // 通知语言特性安装完成
    void installFinished(const LanguageFeatureInstallTask &task,
                         const PackageInstallResult &packageResult);

private Q_SLOTS:
    // 更新软件包安装进度
    void onPackageInstallProgress(const QString &requestId,
                                  const QString &packageName,
                                  int progress,
                                  int state,
                                  const QString &logInfo);
    // 处理软件包安装完成信号
    void onPackageInstallFinished(const QString &requestId,
                                  const QString &packageName,
                                  bool success,
                                  const QString &errorText);
    // 接收软件包卸载进度信号
    void onPackageUninstallProgress(const QString &requestId,
                                    const QString &packageName,
                                    int progress,
                                    int state,
                                    const QString &logInfo);
    // 处理软件包卸载完成信号
    void onPackageUninstallFinished(const QString &requestId,
                                    const QString &packageName,
                                    bool success,
                                    const QString &errorText);

private:
    // 获取语言特性对应的软件包名
    QString packageNameForFeature(const InstallLanguageResult &result,
                                  LanguageFeatureType type) const;
    // 更新已存在的安装任务
    bool updateExistingInstallTask(const LanguageFeatureInstallTask &task);
    // 处理下一个安装任务
    void processNextInstall();
    // 处理下一个卸载任务
    void processNextRemove();
    // 完成指定软件包的安装任务
    void finishActiveInstallPackage(const QString &packageName,
                                    const PackageInstallResult &packageResult);
    // 将当前安装任务全部置为失败
    void failActiveInstallPackages(const QString &errorText);
    // 结束当前安装任务
    void finishInstallJob();
    // 完成指定软件包的卸载任务
    void finishRemovePackage(const PackageInstallResult &packageResult);
    // 结束当前卸载任务
    void finishRemoveJob();

    QQueue<LanguageFeatureInstallJob> m_installQueue;
    QHash<QString, QList<LanguageFeatureInstallTask>> m_activeInstallTasks;
    QString m_activeRequestId;
    bool m_installRunning = false;
    QQueue<QStringList> m_removeQueue;
    QString m_activeRemoveRequestId;
    QStringList m_activeRemovePackageNames;
    bool m_removeRunning = false;
};

#endif // LANGUAGEINSTALL_H
