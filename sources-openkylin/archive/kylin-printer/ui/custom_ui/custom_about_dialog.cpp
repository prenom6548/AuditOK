/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "custom_about_dialog.h"
#include "global_variable.h"
#include "base_info.h"
#include <QProcess>
#include "common.h"

CustomAboutDialog::CustomAboutDialog(QWidget *parent) :
    kdk::KAboutDialog(parent)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setAppName(BaseInfo::instance().getAppDisplayName());
    setAppIcon(QIcon::fromTheme(APP_ICON_NAME));
    setAppVersion(tr("Version: ") + getAppVersion());
}

CustomAboutDialog::~CustomAboutDialog()
{

}

QString CustomAboutDialog::getAppVersion()
{
    return getPackageVersion(APP_NAME);
}
