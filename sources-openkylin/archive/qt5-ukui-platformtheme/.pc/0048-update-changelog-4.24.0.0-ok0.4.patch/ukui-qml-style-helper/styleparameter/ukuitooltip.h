/*
 * Qt5-UKUI's Library
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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef UKUITOOLTIP_H
#define UKUITOOLTIP_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIToolTip : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)
    Q_PROPERTY(QBrush backColor READ backColor WRITE setBackColor NOTIFY backColorChanged)
    Q_PROPERTY(QBrush backBorderColor READ backBorderColor WRITE setBackBorderColor NOTIFY backBorderColorChanged)
    Q_PROPERTY(QBrush shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(QBrush textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(int margins READ margins WRITE setMargins NOTIFY marginsChanged)
    Q_PROPERTY(QBrush backTransparentColor READ backTransparentColor WRITE setBackTransparentColor NOTIFY backTransparentColorChanged)
    Q_PROPERTY(QBrush backBorderTransparentColor READ backTransparentBorderColor WRITE setBackTransparentBorderColor NOTIFY backTransparentBorderColorChanged)

public:
    explicit UKUIToolTip(QQuickItem *parent = nullptr);
    ~UKUIToolTip();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUIToolTip* qmlAttachedProperties(QObject* parent);

    int padding() const;
    void setPadding(int newPadding);

    const QBrush &backColor() const;
    void setBackColor(const QBrush &newBackColor);

    const QBrush &backBorderColor() const;
    void setBackBorderColor(const QBrush &newBackBorderColor);

    const QBrush &shadowColor() const;
    void setShadowColor(const QBrush &newShadowColor);

    const QBrush &textColor() const;
    void setTextColor(const QBrush &newTextColor);

    int radius() const;
    void setRadius(int newRadius);

    int margins() const;
    void setMargins(int newMargins);

    const QBrush &backTransparentColor() const;
    void setBackTransparentColor(const QBrush &newBackTransparentColor);

    const QBrush &backTransparentBorderColor() const;
    void setBackTransparentBorderColor(const QBrush &newBackBorderTransparentColor);

signals:
    void paddingChanged();

    void backColorChanged();

    void backBorderColorChanged();

    void shadowColorChanged();

    void textColorChanged();

    void radiusChanged();

    void marginsChanged();

    void parametryChanged();

    void backTransparentColorChanged();

    void backTransparentBorderColorChanged();

private:
    int m_padding = 12;
    Q_INVOKABLE QBrush m_backColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.85));
    Q_INVOKABLE QBrush m_backBorderColor = QBrush(QColor::fromRgbF(1, 1, 1));
    Q_INVOKABLE QBrush m_shadowColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.3));
    Q_INVOKABLE QBrush m_textColor;
    int m_radius = 6;
    int m_margins = 6;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    QBrush m_backTransparentColor;
    QBrush m_backBorderTransparentColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIToolTip, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUITOOLTIP_H
