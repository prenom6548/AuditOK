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
#include "custom_about_dialog.h"
#include "base_info.h"
#include "common.h"

CustomAboutDialog::CustomAboutDialog(QWidget *parent) :
    kdk::KAboutDialog(parent)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setAppName(BaseInfo::instance().getAppDisplayName());
    setAppIcon(QIcon::fromTheme("kylin-scanner"));
    setAppVersion(tr("Version: ") + getAppVersion());
}

CustomAboutDialog::~CustomAboutDialog()
{
}

QString CustomAboutDialog::getAppVersion()
{
    return getPackageVersion("kylin-scanner");
}
