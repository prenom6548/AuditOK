/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QObject>
#include "QGSettings/qgsettings.h"
#include <QPalette>
class StyleHelper : public QObject
{
    Q_OBJECT
public:
    explicit StyleHelper(QObject *parent = nullptr);
    static StyleHelper *instance();

    QColor getColor(QPalette::ColorRole role = QPalette::Base, QPalette::ColorGroup group = QPalette::Active);
    qreal getOpacity() const;
    qreal getFontSize() const;
    bool isLightStyle() const;
private:
    void initStyleSettings();

private:
    QGSettings* m_styleSettings = nullptr;
    QGSettings* m_opacitySettings = nullptr;

    qreal m_opacity;
    qreal m_fontSize;
    bool m_isLightStyle;
    bool m_isTabletMode;
};

#endif // COLORHELPER_H
