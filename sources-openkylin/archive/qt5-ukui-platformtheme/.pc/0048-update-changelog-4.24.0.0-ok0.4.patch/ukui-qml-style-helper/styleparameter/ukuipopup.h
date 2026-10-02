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

#ifndef UKUIPOPUP_H
#define UKUIPOPUP_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIPopup : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)

    Q_PROPERTY(QBrush normalColor READ normalColor WRITE setNormalColor NOTIFY normalColorChanged)
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)
    Q_PROPERTY(QBrush backColor READ backColor WRITE setBackColor NOTIFY backColorChanged)
    Q_PROPERTY(QBrush backBorderColor READ backBorderColor WRITE setBackBorderColor NOTIFY backBorderColorChanged)
    Q_PROPERTY(QBrush shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(QBrush transparentBC READ transparentBC WRITE setTransparentBC NOTIFY transparentBCChanged)
    Q_PROPERTY(QBrush transparentBorderColor READ transparentBorderColor WRITE setTransparentBorderColor NOTIFY transparentBorderColorChanged)

public:
    explicit UKUIPopup(QQuickItem *parent = nullptr);
    ~UKUIPopup();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUIPopup* qmlAttachedProperties(QObject* parent);

    const QBrush &normalColor() const;
    void setNormalColor(const QBrush &newNormalColor);

    int padding() const;
    void setPadding(int newPadding);

    const QBrush &backColor() const;
    void setBackColor(const QBrush &newBackColor);

    const QBrush &backBorderColor() const;
    void setBackBorderColor(const QBrush &newBackBorderColor);

    const QBrush &shadowColor() const;
    void setShadowColor(const QBrush &newShadowColor);

    int radius() const;
    void setRadius(int newRadius);

    const QBrush &transparentBC() const;
    void setTransparentBC(const QBrush &newTransparentBC);

    const QBrush &transparentBorderColor() const;
    void setTransparentBorderColor(const QBrush &newTransparentBorderColor);

signals:
    void normalColorChanged();

    void paddingChanged();

    void backColorChanged();

    void backBorderColorChanged();

    void shadowColorChanged();

    void radiusChanged();

    void parametryChanged();

    void transparentBCChanged();

    void transparentBorderColorChanged();

private:
    Q_INVOKABLE QBrush m_normalColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.85));

    int m_padding = 8;
    Q_INVOKABLE QBrush m_backColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.85));
    Q_INVOKABLE QBrush m_backBorderColor = QBrush(QColor::fromRgbF(1, 1, 1));
    Q_INVOKABLE QBrush m_shadowColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.3));
    int m_radius;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    QBrush m_transparentBC;
    QBrush m_transparentBorderColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIPopup, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUIPOPUP_H
