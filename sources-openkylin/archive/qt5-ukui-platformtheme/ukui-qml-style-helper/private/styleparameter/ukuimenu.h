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

#ifndef UKUIMENU_H
#define UKUIMENU_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIMenu : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int leftRightPadding READ leftRightPadding WRITE setLeftRightPadding NOTIFY leftRightPaddingChanged)
    Q_PROPERTY(int topBottomPadding READ topBottomPadding WRITE setTopBottomPadding NOTIFY topBottomPaddingChanged)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(int border READ border WRITE setBorder NOTIFY borderChanged)
    Q_PROPERTY(QBrush  shadowNormalColor READ shadowNormalColor WRITE setshadowNormalColor NOTIFY shadowNormalColorChanged)
    Q_PROPERTY(QBrush  shadowDisableColor READ shadowDisableColor WRITE setshadowDisableColor NOTIFY shadowDisableColorChanged)
    Q_PROPERTY(QBrush  normalBC READ normalBC WRITE setNormalBC NOTIFY normalBCChanged)
    Q_PROPERTY(QBrush  normalBorderColor READ normalBorderColor WRITE setNormalBorderColor NOTIFY normalBorderColorChanged)
    Q_PROPERTY(double bcColorAlpha READ bcColorAlpha WRITE setBcColorAlpha NOTIFY bcColorAlphaChanged)

    Q_PROPERTY(QBrush  normalTransparentBC READ normalBC WRITE setNormalTransparentBC NOTIFY normalTransparentBCChanged)
    Q_PROPERTY(QBrush  normalTransparentBorderColor READ normalTransparentBorderColor WRITE setNormalTransparentBorderColor NOTIFY normalTransparentBorderColorChanged)

public:
    explicit UKUIMenu(QQuickItem *parent = nullptr);
    ~UKUIMenu();
    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUIMenu* qmlAttachedProperties(QObject* parent);

    int leftRightPadding() const;
    void setLeftRightPadding(int newLeftRightPadding);

    int topBottomPadding() const;
    void setTopBottomPadding(int newTopBottomPadding);

    int radius() const;
    void setRadius(int newRadius);

    const QBrush  &shadowNormalColor() const;
    void setshadowNormalColor(const QBrush  &newshadowNormalColor);

    const QBrush  &shadowDisableColor() const;
    void setshadowDisableColor(const QBrush  &newShadowDisableColor);

    const QBrush  &normalBC() const;
    void setNormalBC(const QBrush  &newNormalBC);

    const QBrush  &normalBorderColor() const;
    void setNormalBorderColor(const QBrush  &newNormalBorderColor);

    int border() const;
    void setBorder(int newBorder);

    double bcColorAlpha() const;
    void setBcColorAlpha(double newBcColorAlpha);

    void setNormalTransparentBC(const QBrush &newNormalTransparentBC);

    const QBrush &normalTransparentBorderColor() const;
    void setNormalTransparentBorderColor(const QBrush &newNormalTransparentBorderColor);

signals:
    void leftRightPaddingChanged();

    void topBottomPaddingChanged();

    void radiusChanged();

    void shadowNormalColorChanged();

    void shadowDisableColorChanged();

    void normalBCChanged();

    void normalBorderColorChanged();

    void borderChanged();

    void parametryChanged();

    void bcColorAlphaChanged();

    void normalTransparentBCChanged();

    void normalTransparentBorderColorChanged();

private:
    int m_leftRightPadding = 8;
    int m_topBottomPadding = 8;
    int m_radius = 8;
    Q_INVOKABLE QBrush  m_shadowNormalColor = QBrush(QColor::fromRgbF(0, 0, 0, 0.3));
    Q_INVOKABLE QBrush  m_shadowDisableColor;
    Q_INVOKABLE QBrush  m_normalBC;
    Q_INVOKABLE QBrush  m_normalBorderColor;
    int m_border;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

    double m_bcColorAlpha;
    QBrush m_normalTransparentBC;
    QBrush m_normalTransparentBorderColor;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIMenu, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUIMENU_H
