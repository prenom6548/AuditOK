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
#ifndef BATCHIMPORTDIALOG_H
#define BATCHIMPORTDIALOG_H
#include <kaboutdialog.h>
#include "../fileProcess/fileprocess.h"

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QUrl>
#include <QDesktopServices>

class BatchImportDialog : public kdk::KDialog
{
    Q_OBJECT
public:
    explicit BatchImportDialog(QWidget *parent = nullptr);

signals:
    void startimport();
};



#endif // BATCHIMPORTDIALOG_H
