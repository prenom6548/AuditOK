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

#include "ukuipopup.h"
#include "qdebug.h"
#include "../../../libqt5-ukui-style/settings/ukui-style-settings.h"

using namespace UKUIQQC2Style;

UKUIPopup::UKUIPopup(QQuickItem *parent) : QQuickItem(parent)
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
    }
}

UKUIPopup::~UKUIPopup()
{

}

void UKUIPopup::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setBackColor(instance->kMenu());
    setBackBorderColor(instance->kLineWindowActive());
    setRadius(instance->kradiusMenu());
    setShadowColor(instance->shadowActive());
    QColor c = instance->kMenu().color();
    if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
        auto opacity = UKUIStyleSettings::globalInstance()->get("menuTransparency").toInt()/100.0;
        c.setAlphaF(opacity);
    }
    setTransparentBC(c);
    setTransparentBorderColor(QBrush(QColor(0,0,0,0)));
    emit parametryChanged();

}

UKUIPopup* UKUIPopup::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIPopup(p);
}

const QBrush &UKUIPopup::normalColor() const
{
    return m_normalColor;
}

void UKUIPopup::setNormalColor(const QBrush &newNormalColor)
{
    if (m_normalColor == newNormalColor)
        return;
    m_normalColor = newNormalColor;
    emit normalColorChanged();
}


int UKUIPopup::padding() const
{
    return m_padding;
}

void UKUIPopup::setPadding(int newPadding)
{
    if (m_padding == newPadding)
        return;
    m_padding = newPadding;
    emit paddingChanged();
}

const QBrush &UKUIPopup::backColor() const
{
    return m_backColor;
}

void UKUIPopup::setBackColor(const QBrush &newBackColor)
{
    if (m_backColor == newBackColor)
        return;
    m_backColor = newBackColor;
    emit backColorChanged();
}

const QBrush &UKUIPopup::backBorderColor() const
{
    return m_backBorderColor;
}

void UKUIPopup::setBackBorderColor(const QBrush &newBackBorderColor)
{
    if (m_backBorderColor == newBackBorderColor)
        return;
    m_backBorderColor = newBackBorderColor;
    emit backBorderColorChanged();
}

const QBrush &UKUIPopup::shadowColor() const
{
    return m_shadowColor;
}

void UKUIPopup::setShadowColor(const QBrush &newShadowColor)
{
    if (m_shadowColor == newShadowColor)
        return;
    m_shadowColor = newShadowColor;
    emit shadowColorChanged();
}

int UKUIPopup::radius() const
{
    return m_radius;
}

void UKUIPopup::setRadius(int newRadius)
{
    if (m_radius == newRadius)
        return;
    m_radius = newRadius;
    emit radiusChanged();
}

const QBrush &UKUIPopup::transparentBC() const
{
    return m_transparentBC;
}

void UKUIPopup::setTransparentBC(const QBrush &newTransparentBC)
{
    if (m_transparentBC == newTransparentBC)
        return;
    m_transparentBC = newTransparentBC;
    emit transparentBCChanged();
}

const QBrush &UKUIPopup::transparentBorderColor() const
{
    return m_transparentBorderColor;
}

void UKUIPopup::setTransparentBorderColor(const QBrush &newTransparentBorderColor)
{
    if (m_transparentBorderColor == newTransparentBorderColor)
        return;
    m_transparentBorderColor = newTransparentBorderColor;
    emit transparentBorderColorChanged();
}
