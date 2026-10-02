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
#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <kaboutdialog.h>

#include <QObject>
#include <QWidget>

class AboutDialog : public kdk::KAboutDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);
    QString getCurrentVersion();
signals:

};

#endif // ABOUTDIALOG_H
