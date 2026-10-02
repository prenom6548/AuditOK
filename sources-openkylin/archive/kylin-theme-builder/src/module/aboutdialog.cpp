/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
        kdk::KAboutDialog(parent)
{
    this->setAppIcon(QIcon::fromTheme("kylin-theme-builder"));
    this->setAppName(tr("Kylin Theme Builder"));
    this->setAppVersion(this->getCurrentVersion());
    this->setBodyText(tr(""));
}

QString AboutDialog::getCurrentVersion()
{
    //use self define main version
    //return VERSION;

    FILE *pp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *q = NULL;
    QString version;

    //fix bug#125289, use dpkg query version instead of self define version
    pp = popen("dpkg -l kylin-theme-builder", "r");
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
