/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "displaywidget.h"
#include <QDebug>

DisplayWidget::DisplayWidget(QWidget *parent) :
    QWidget(parent),
    m_defaultConnectDetectPageWidget(new DetectPageWidget(this)),
    m_defaultConnectFailedPageWidget(new FailedPageWidget(this)),
    m_defaultConnectSuccessPageWidget(new SuccessPageWidget(this)),
    m_displayStackedLayout(new QStackedLayout(this))
{
    setupGui();
    initConnect();
}

void DisplayWidget::setupGui()
{
    m_displayStackedLayout->addWidget(m_defaultConnectDetectPageWidget);
    m_displayStackedLayout->addWidget(m_defaultConnectFailedPageWidget);
    m_displayStackedLayout->addWidget(m_defaultConnectSuccessPageWidget);

    m_displayStackedLayout->setContentsMargins(0, 0, 0, 0);
    m_displayStackedLayout->setCurrentWidget(m_defaultConnectDetectPageWidget);
}

void DisplayWidget::initConnect()
{
    connect(m_defaultConnectFailedPageWidget, &FailedPageWidget::scanButtonClicked, this, &DisplayWidget::showDetectPageSlot);
    connect(g_user_signal, &GlobalUserSignal::switchToDetectPageSignal, this, &DisplayWidget::showDetectPageSlot);
    connect(g_user_signal, &GlobalUserSignal::refreshListInFilePage, this, &DisplayWidget::refreshListSlot);
}

void DisplayWidget::refreshListSlot(){
    //    QProcess proc;
    //    QString cmd = "pkexec udevadm control --reload";
    //    QString cmd2 = "pkexec udevadm trigger --action=add";
    //    proc.start(cmd);
    //    proc.waitForFinished();
    //    proc.start(cmd2);
    //    proc.waitForFinished();
    createAndExecuteScript();
    QProcess::startDetached(qApp->applicationFilePath());
    qApp->quit();

}

void DisplayWidget::showDetectPageSlot()
{
    m_displayStackedLayout->setCurrentWidget(m_defaultConnectDetectPageWidget);
    g_user_signal->setIsFailPage(false);

    emit detectScanDevicesSignal();
}

void DisplayWidget::showSuccessPageSlot(bool isSucceed)
{
    m_displayStackedLayout->setCurrentWidget(m_defaultConnectSuccessPageWidget);
    g_user_signal->setIsFailPage(false);
    m_defaultConnectSuccessPageWidget->updateScanSettingsSlot(isSucceed);
}

void DisplayWidget::showFailedPageSlot()
{
    m_displayStackedLayout->setCurrentWidget(m_defaultConnectFailedPageWidget);
    g_user_signal->setIsFailPage(true);
}

void DisplayWidget::showSuccessImageHandlePageSlot()
{
    m_displayStackedLayout->setCurrentWidget(m_defaultConnectSuccessPageWidget);
    g_user_signal->setIsFailPage(false);
    m_defaultConnectSuccessPageWidget->showLeftImageHandleSuccessPage();
}

void DisplayWidget::createAndExecuteScript() {
    // 脚本文件路径
    QDir cacheDir(QDir::homePath() + "/.cache");
    QString scriptPath = cacheDir.filePath("udev_reload_trigger.sh");

    // 检查脚本文件是否存在
    if (!QFile::exists(scriptPath)) {
        // 脚本内容
        QString scriptContent = R"(#!/bin/bash
udevadm control --reload
udevadm trigger --action=add
rm "$0")";

        // 创建脚本文件并写入内容
        QFile scriptFile(scriptPath);
        if (scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&scriptFile);
            out << scriptContent;
            scriptFile.close();

            // 设置脚本为可执行
            if (!scriptFile.setPermissions(QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther)) {
                qWarning() << "Failed to set executable permissions for script:" << scriptPath;
                return;
            }

        } else {
            qWarning() << "Failed to create script file:" << scriptPath;
            return;
        }
    }

    // 执行脚本
    QProcess proc;
    proc.start("pkexec", QStringList() << scriptPath);
    proc.waitForFinished();

    // 检查执行结果
    QString output = proc.readAllStandardOutput();
    QString error = proc.readAllStandardError();
    if (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0) {
        qDebug() << "Script executed successfully.";
    } else {
        qWarning() << "Script execution failed:" << error;
    }
}
