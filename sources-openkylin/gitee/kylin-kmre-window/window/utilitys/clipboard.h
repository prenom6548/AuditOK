/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QClipboard>
#include <QVariant>
#include "singleton.h"


class Clipboard : public QObject, public kmre::Singleton<Clipboard>
{
    Q_OBJECT

public:
    void clear();
    bool getImage(QVariant &imageData);

public slots:
    void onSyncClipboardFromAndroid(const QString &content);

private:
    Clipboard();
    ~Clipboard(){}

private slots:
    void onClipboardDataChanged();

private:
    QClipboard *mClipboard = nullptr;

    friend Singleton<Clipboard>;
};