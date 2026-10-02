/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef KSHADOWHELPER_H
#define KSHADOWHELPER_H

#include "gui_g.h"
#include <QObject>
#include <QWidget>
namespace kdk
{
namespace effects
{

/**
 * @since 2.0.2.1
 */

class KShadowHelperPrivate;

class KShadowHelper : public QObject
{
    Q_OBJECT
public:
    static KShadowHelper *self();
    void setWidget(QWidget *widget, int borderRadius = 12, int shadowWidth = 20, qreal darkness = 0.5);

private:
    KShadowHelper(QObject *parent = nullptr);
    Q_DECLARE_PRIVATE(KShadowHelper)
    KShadowHelperPrivate *const d_ptr;
};
}
}
#endif // KSHADOWHELPER_H
