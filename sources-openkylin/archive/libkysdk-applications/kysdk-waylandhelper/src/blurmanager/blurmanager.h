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

#ifndef BLURMANAGER_H
#define BLURMANAGER_H

#include <QObject>
#include <QRegion>
#include <QWindow>

#include "blurabstractinterface.h"

/**
 * @since 2.5.1.0
 */

namespace kdk
{
class BlurManagerPrivate;

class Q_DECL_EXPORT BlurManager : public QObject
{
    Q_OBJECT
public:
    static BlurManager *self();
    static bool setBlurBehindWithStrength(QWindow *window, bool enable = true, const QRegion &region = QRegion(), uint32_t strength = -1);

private:
    BlurManager(QObject *parent = nullptr);
    BlurAbstractInterface *interface();
    BlurAbstractInterface *m_blurInterface = nullptr;
};

}

#endif // BLURMANAGER_H
