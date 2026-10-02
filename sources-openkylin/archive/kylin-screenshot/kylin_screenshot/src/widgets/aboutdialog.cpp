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
#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
   : kdk::KAboutDialog(parent)
{
    setAppIcon(QIcon::fromTheme("kylin-screenshot"));
    setAppName(tr("screenshot"));
    setAppVersion(tr("Version: ") + getScreenshotVersion());
    this->closeButton()->setToolTip(tr("Close"));
    setAccessibleName("kylin-screenshot_KAboutDialog_aboutDialog");
    closeButton()->setAccessibleName("kylin-screenshot_QPushButton_closeButton");
    maximumButton()->setAccessibleName("kylin-screenshot_QPushButton_maximumButton");
    minimumButton()->setAccessibleName("kylin-screenshot_QPushButton_minimumButton");
    menuButton()->setAccessibleName("kylin-screenshot_KMenuButton_menuButtonn");
    mainWidget()->setAccessibleName("kylin-screenshot_QWidget_mainWidget");
}

QString AboutDialog::getScreenshotVersion()
{
    FILE *pp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *q = NULL;
    QString version = tr("none");
    pp = popen("dpkg -l kylin-screenshot", "r");
    if(NULL == pp)
        return version;

    while((read = getline(&line, &len, pp)) != -1){
        q = strrchr(line, '\n');
        *q = '\0';
        QString content = line;
        QStringList list = content.split(" ");
        list.removeAll("");
        if (list.size() >= 3)
            version = list.at(2);
    }
    free(line);
    pclose(pp);
    return version;
}
