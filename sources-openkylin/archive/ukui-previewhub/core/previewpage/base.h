/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co, Ltd.
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
 * Authors: Lu Hongxu <lhongxu@outlook.com>
 *
 */

#ifndef BASEPREVIEWPAGE_H
#define BASEPREVIEWPAGE_H

#include <QFrame>
#include <QObject>
#include <QWidget>
#include <QLayout>
#include <QDebug>

#include "previewhub_fileinfo.h"

class BasePreviewPage : public QFrame
{
    Q_OBJECT
public:
    BasePreviewPage(QWidget *parent = nullptr);
    BasePreviewPage() = delete;
    ~BasePreviewPage() = default;
    virtual void cancel() = 0;

protected:
    QVBoxLayout *base_layout;

public Q_SLOTS:
    virtual void updateInfo(PreviewHubFileInfo *info) = 0;
};

#endif // BASEPREVIEWPAGE_H
