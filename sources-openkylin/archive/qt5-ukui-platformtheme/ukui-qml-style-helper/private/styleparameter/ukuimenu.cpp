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
 */

#include "ukuimenu.h"
#include "qdebug.h"
#include "../../../libqt5-ukui-style/settings/ukui-style-settings.h"

using namespace UKUIQQC2Style;

UKUIMenu::UKUIMenu(QQuickItem *parent) : QQuickItem(parent)
{
    if(!qApp || !qApp->property("qqc2-globaltoken").isValid())
        return;
    TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
    if(token && token->getInstance()){
        m_instance = token->getInstance();
        initParam(m_instance);
        connect(m_instance, &UKUIGlobalDTConfig::GlobalDTConfig::tokenChanged, this, [=](){
            initParam(m_instance);
        }, Qt::UniqueConnection);
        if (QGSettings::isSchemaInstalled("org.ukui.style")) {
            auto settings = UKUIStyleSettings::globalInstance();
            connect(settings, &QGSettings::changed, this, [&](const QString& key){
                if(key == "menuTransparency" || key == "menu-transparency")
                    initParam(m_instance);
            }, Qt::UniqueConnection);
        }
    }
}

UKUIMenu::~UKUIMenu()
{

}

void UKUIMenu::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setshadowNormalColor(instance->shadowActive());
    setshadowDisableColor(instance->shadowDisable());
    setNormalBorderColor(instance->kLineWindowActive());
    setBorder(instance->normalline());
    setRadius(instance->kradiusMenu());
    QColor c = instance->kMenu().color();
    if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
        auto opacity = UKUIStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
        c.setAlphaF(opacity);
        setBcColorAlpha(opacity);
    }
    setNormalBC(instance->kMenu());
    setNormalTransparentBC(c);
    setNormalTransparentBorderColor(instance->kLineWindowActive());

    emit parametryChanged();

}

UKUIMenu* UKUIMenu::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIMenu(p);
}

int UKUIMenu::leftRightPadding() const
{
    return m_leftRightPadding;
}

void UKUIMenu::setLeftRightPadding(int newLeftRightPadding)
{
    if (m_leftRightPadding == newLeftRightPadding)
        return;
    m_leftRightPadding = newLeftRightPadding;
    emit leftRightPaddingChanged();
}

int UKUIMenu::topBottomPadding() const
{
    return m_topBottomPadding;
}

void UKUIMenu::setTopBottomPadding(int newTopBottomPadding)
{
    if (m_topBottomPadding == newTopBottomPadding)
        return;
    m_topBottomPadding = newTopBottomPadding;
    emit topBottomPaddingChanged();
}


int UKUIMenu::radius() const
{
    return m_radius;
}

void UKUIMenu::setRadius(int newRadius)
{
    if (m_radius == newRadius)
        return;
    m_radius = newRadius;
    emit radiusChanged();
}


const QBrush  &UKUIMenu::shadowNormalColor() const
{
    return m_shadowNormalColor;
}

void UKUIMenu::setshadowNormalColor(const QBrush  &newshadowNormalColor)
{
    if (m_shadowNormalColor == newshadowNormalColor)
        return;
    m_shadowNormalColor = newshadowNormalColor;
    emit shadowNormalColorChanged();
}

const QBrush  &UKUIMenu::shadowDisableColor() const
{
    return m_shadowDisableColor;
}

void UKUIMenu::setshadowDisableColor(const QBrush  &newShadowDisableColor)
{
    if (m_shadowDisableColor == newShadowDisableColor)
        return;
    m_shadowDisableColor = newShadowDisableColor;
    emit shadowDisableColorChanged();
}

const QBrush  &UKUIMenu::normalBC() const
{
    return m_normalBC;
}

void UKUIMenu::setNormalBC(const QBrush  &newNormalBC)
{
    if (m_normalBC == newNormalBC)
        return;
    m_normalBC = newNormalBC;
    emit normalBCChanged();
}

const QBrush  &UKUIMenu::normalBorderColor() const
{
    return m_normalBorderColor;
}

void UKUIMenu::setNormalBorderColor(const QBrush  &newNormalBorderColor)
{
    if (m_normalBorderColor == newNormalBorderColor)
        return;
    m_normalBorderColor = newNormalBorderColor;
    emit normalBorderColorChanged();
}

int UKUIMenu::border() const
{
    return m_border;
}

void UKUIMenu::setBorder(int newBorder)
{
    if (m_border == newBorder)
        return;
    m_border = newBorder;
    emit borderChanged();
}

double UKUIMenu::bcColorAlpha() const
{
    return m_bcColorAlpha;
}

void UKUIMenu::setBcColorAlpha(double newBcColorAlpha)
{
    if (qFuzzyCompare(m_bcColorAlpha, newBcColorAlpha))
        return;
    m_bcColorAlpha = newBcColorAlpha;
    emit bcColorAlphaChanged();
}

void UKUIMenu::setNormalTransparentBC(const QBrush &newNormalTransparentBC)
{
    if (m_normalTransparentBC == newNormalTransparentBC)
        return;
    m_normalTransparentBC = newNormalTransparentBC;
    emit normalTransparentBCChanged();
}

const QBrush &UKUIMenu::normalTransparentBorderColor() const
{
    return m_normalTransparentBorderColor;
}

void UKUIMenu::setNormalTransparentBorderColor(const QBrush &newNormalTransparentBorderColor)
{
    if (m_normalTransparentBorderColor == newNormalTransparentBorderColor)
        return;
    m_normalTransparentBorderColor = newNormalTransparentBorderColor;
    emit normalTransparentBorderColorChanged();
}
