/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FILEEVENTHANDLERIMPL_H
#define FILEEVENTHANDLERIMPL_H

#include <QObject>
#include "rep_fileeventhandler_source.h"

class FileEventHandlerImpl : public FileEventHandlerSource
{
    Q_OBJECT
public:
    explicit FileEventHandlerImpl(QObject *parent = nullptr);

Q_SIGNALS:
    void reciveClientFileEvent(int eventType, const QString &arg1, const QString &arg2);

public Q_SLOTS:
    virtual void handleFileEvent(int eventType, QString arg1, QString arg2);
};

#endif // FILEEVENTHANDLERIMPL_H
