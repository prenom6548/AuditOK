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

#include "ukuitabbutton.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUITabButton::UKUITabButton(QQuickItem *parent) : QQuickItem(parent)
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

UKUITabButton::~UKUITabButton()
{

}

void UKUITabButton::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalColor(QBrush(QColor(0,0,0,0)));
    setHoverColor(instance->kContainHover());
    setClickedColor(instance->kContainClick());
    setCheckedColor(instance->kContainGeneralNormal());
    setBorderWidth(1);
    setNormalBorderColor(instance->kLineComponentNormal());
    setHoverBorderColor(instance->kLineComponentHover());
    setClickBorderColor(instance->kLineComponentClick());
    setCheckedBorderColor(instance->kLineComponentNormal());
    setNormalTextColor(instance->kFontPrimary());

    setNormalTransparentColor(QBrush(QColor(0,0,0,0)));
    setHoverTransparentColor(instance->kContainAlphaHover());
    setClickedTransparentColor(instance->kContainAlphaClick());
    setCheckedTransparentColor(instance->kContainGeneralNormal());

    setFocusBorderColor(instance->kBrandFocus());
    emit parametryChanged();

}

UKUITabButton* UKUITabButton::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUITabButton(p);
}

const QBrush &UKUITabButton::normalColor() const
{
    return m_normalColor;
}

void UKUITabButton::setNormalColor(const QBrush &newNormalColor)
{
    if (m_normalColor == newNormalColor)
        return;
    m_normalColor = newNormalColor;
    emit normalColorChanged();
}

const QBrush &UKUITabButton::hoverColor() const
{
    return m_hoverColor;
}

void UKUITabButton::setHoverColor(const QBrush &newHoverColor)
{
    if (m_hoverColor == newHoverColor)
        return;
    m_hoverColor = newHoverColor;
    emit hoverColorChanged();
}

const QBrush &UKUITabButton::checkedColor() const
{
    return m_checkedColor;
}

void UKUITabButton::setCheckedColor(const QBrush &newCheckedColor)
{
    if (m_checkedColor == newCheckedColor)
        return;
    m_checkedColor = newCheckedColor;
    emit checkedColorChanged();
}

const QBrush &UKUITabButton::clickedColor() const
{
    return m_clickedColor;
}

void UKUITabButton::setClickedColor(const QBrush &newClickedColor)
{
    if (m_clickedColor == newClickedColor)
        return;
    m_clickedColor = newClickedColor;
    emit clickedColorChanged();
}

int UKUITabButton::borderWidth() const
{
    return m_borderWidth;
}

void UKUITabButton::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

const QBrush &UKUITabButton::normalBorderColor() const
{
    return m_normalBorderColor;
}

void UKUITabButton::setNormalBorderColor(const QBrush &newNormalBorderColor)
{
    if (m_normalBorderColor == newNormalBorderColor)
        return;
    m_normalBorderColor = newNormalBorderColor;
    emit normalBorderColorChanged();
}

const QBrush &UKUITabButton::hoverBorderColor() const
{
    return m_hoverBorderColor;
}

void UKUITabButton::setHoverBorderColor(const QBrush &newHoverBorderColor)
{
    if (m_hoverBorderColor == newHoverBorderColor)
        return;
    m_hoverBorderColor = newHoverBorderColor;
    emit hoverBorderColorChanged();
}

const QBrush &UKUITabButton::clickBorderColor() const
{
    return m_clickBorderColor;
}

void UKUITabButton::setClickBorderColor(const QBrush &newClickBorderColor)
{
    if (m_clickBorderColor == newClickBorderColor)
        return;
    m_clickBorderColor = newClickBorderColor;
    emit clickBorderColorChanged();
}

const QBrush &UKUITabButton::checkedBorderColor() const
{
    return m_checkedBorderColor;
}

void UKUITabButton::setCheckedBorderColor(const QBrush &newCheckedBorderColor)
{
    if (m_checkedBorderColor == newCheckedBorderColor)
        return;
    m_checkedBorderColor = newCheckedBorderColor;
    emit checkedBorderColorChanged();
}

const QBrush &UKUITabButton::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUITabButton::setNormalTextColor(const QBrush &newNormalTextColor)
{
    if (m_normalTextColor == newNormalTextColor)
        return;
    m_normalTextColor = newNormalTextColor;
    emit disableTextColorChanged();
}

const QBrush &UKUITabButton::normalTransparentColor() const
{
    return m_normalTransparentColor;
}

void UKUITabButton::setNormalTransparentColor(const QBrush &newNormalTransparentColor)
{
    if (m_normalTransparentColor == newNormalTransparentColor)
        return;
    m_normalTransparentColor = newNormalTransparentColor;
    emit normalTransparentColorChanged();
}

const QBrush &UKUITabButton::hoverTransparentColor() const
{
    return m_hoverTransparentColor;
}

void UKUITabButton::setHoverTransparentColor(const QBrush &newHoverTransparentColor)
{
    if (m_hoverTransparentColor == newHoverTransparentColor)
        return;
    m_hoverTransparentColor = newHoverTransparentColor;
    emit hoverTransparentColorChanged();
}

const QBrush &UKUITabButton::checkedTransparentColor() const
{
    return m_checkedTransparentColor;
}

void UKUITabButton::setCheckedTransparentColor(const QBrush &newCheckedTransparentColor)
{
    if (m_checkedTransparentColor == newCheckedTransparentColor)
        return;
    m_checkedTransparentColor = newCheckedTransparentColor;
    emit checkedTransparentColorChanged();
}

const QBrush &UKUITabButton::clickedTransparentColor() const
{
    return m_clickedTransparentColor;
}

void UKUITabButton::setClickedTransparentColor(const QBrush &newClickedTransparentColor)
{
    if (m_clickedTransparentColor == newClickedTransparentColor)
        return;
    m_clickedTransparentColor = newClickedTransparentColor;
    emit clickedTransparentColorChanged();
}

const QBrush &UKUITabButton::focusBorderColor() const
{
    return m_focusBorderColor;
}

void UKUITabButton::setFocusBorderColor(const QBrush &newFocusBorderColor)
{
    if (m_focusBorderColor == newFocusBorderColor)
        return;
    m_focusBorderColor = newFocusBorderColor;
    emit focusBorderColorChanged();
}
