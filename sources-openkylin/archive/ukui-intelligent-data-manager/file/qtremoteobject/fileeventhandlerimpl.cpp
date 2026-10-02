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

#include "fileeventhandlerimpl.h"

#include <QDebug>

FileEventHandlerImpl::FileEventHandlerImpl(QObject *parent) : FileEventHandlerSource(parent)
{

}

void FileEventHandlerImpl::handleFileEvent(int eventType, QString arg1, QString arg2)
{
    qDebug() << "server handle file event" << eventType << arg1 << arg2;
    emit this->reciveClientFileEvent(eventType, arg1, arg2);
}
