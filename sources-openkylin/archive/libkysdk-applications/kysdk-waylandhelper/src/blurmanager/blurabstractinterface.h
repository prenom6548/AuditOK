/*
 * libkysdk-waylandhelper's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef BLURABSTRACTINTERFACE_H
#define BLURABSTRACTINTERFACE_H

#include <QRegion>
#include <QWindow>

/**
 * @brief The BlurAbstractInterface class
 * @since 2.5.1.0
 */
class BlurAbstractInterface : public QObject
{
public:
    BlurAbstractInterface(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    virtual bool setBlurBehindWithStrength(QWindow *window, bool enable = true, const QRegion &region = QRegion(), uint32_t strength = -1) = 0;
};

#endif // BLURABSTRACTINTERFACE_H
