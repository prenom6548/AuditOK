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

#include "aboutdialog.h"
#include "src/common/utils.h"
#include "src/po/translation.h"

#include "KF5/KWindowSystem/kwindowsystem.h"
#include "v11sdk.h"
#include "version.h"

#include <QRegularExpression>


AboutDialog::AboutDialog(QWidget *parent) :
    kdk::KAboutDialog(parent)
{
    setAppIcon(QIcon::fromTheme("burner"));
    setAppName(_("Burner"));
    QString replace_version = replaceVersionString(QString(PROJECT_VERSION));
    setAppVersion(_("Version") + ": " + replace_version);
    setAccessibleName("kylin-burner_KAboutDialog_AboutDialog");
    this->setFixedSize(450,336);
    this->setModal(true);
    this->closeButton()->setToolTip(_("Close"));
    if (SystemInfo::m_isWayland) {
       ukuiWaylandWindowStyleChange(this);
    } else {
        KWindowSystem::setState(this->winId(), NET::SkipPager | NET::SkipSwitcher | NET::SkipTaskbar);
    }
}

AboutDialog::~AboutDialog()
{
}

// 替换版本号字符串的核心函数
QString AboutDialog::replaceVersionString(QString input)
{
#if KAIMING_BUILD
    input = QString(qgetenv("APP_VERSION"));
    return input;
#else
    return input;
#endif
}

