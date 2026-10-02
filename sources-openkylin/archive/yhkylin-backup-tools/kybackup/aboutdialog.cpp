/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "aboutdialog.h"
#include <QRegularExpression>
#include "globalbackupinfo.h"
#include "../common/utils.h"

// #define SUPPORT "<a href=\"mailto://support@kylinos.cn\">support@kylinos.cn</a>"
#define SUPPORT "<u>support@kylinos.cn</u>"

AboutDialog::AboutDialog(QWidget *parent) :
    kdk::KAboutDialog(parent)
{
    //setWindowIcon(QIcon::fromTheme(THEME_YHKYLIN_BACKUP_TOOLS, QIcon(":/images/yhkylin-backup-tools.png")));
    setAppIcon(QIcon::fromTheme(THEME_YHKYLIN_BACKUP_TOOLS, QIcon(":/images/yhkylin-backup-tools.png")));
    //setWindowTitle(tr("Backup & Restore"));
    setAppName(tr("Backup & Restore"));
    setAppVersion(tr("version:") + Utils::getBackupVersion());
    // 麒麟备份还原工具是一款支持系统备份还原和数据备份还原的工具，当用户数据损坏或系统遭受攻击时能够通过该工具灵活的还原到备份节点的状态。针对国产软硬件平台开展了大量的优化和创新。
    setBodyText(tr("The backup tool is a tool that supports system backup and data backup. "
                   "When the user data is damaged or the system is attacked, the tool can flexibly restore "
                   "the status of the backup node. A lot of optimization and innovation have been "
                   "carried out for domestic hardware and software platforms."));
    // setBodyTextVisiable(true);
    // setAppSupport(tr("Service & Support: %1").arg(SUPPORT));

    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, [=](){
        this->setAppIcon(QIcon::fromTheme(THEME_YHKYLIN_BACKUP_TOOLS, QIcon(":/images/yhkylin-backup-tools.png")));
    });
}

AboutDialog::~AboutDialog()
{
}

