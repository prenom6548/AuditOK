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

#include "ukuislider.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUISlider::UKUISlider(QQuickItem *parent) : QQuickItem(parent)
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

UKUISlider::~UKUISlider()
{

}

void UKUISlider::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    if(!instance && qApp->property("qqc2-globaltoken").isValid()){
        TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
        if(token){
            instance = token->getInstance();
        }
    }
    setNormalWidth(240);
    setNormalHeight(24);
    setGrooveHeight(4);
    setHandleHeight(16);
    setHandleHoverHeight(20);

    setTouchNormalWidth(240);
    setTouchNormalHeight(16);
    settouchHandleHeight(16);
    setTouchGrooveHeight(16);
    setTouchHandleWidth(32);
    setTouchHandleColor(instance->kFontWhite());

    setFocusBorderWidth(instance->focusline());

    setFocusBorderColor(instance->kBrandFocus());

    setNormalGrooveColor(instance->kBrandNormal());
    setDisableGrooveColor(instance->kBrandNormal());
    setNormalGrooveBorderColor(QBrush(QColor(0,0,0,0)));
    setDisableGrooveBorderColor(QBrush(QColor(0,0,0,0)));
    setGrooveBorderWidth(0);

    setNormalUnGrooveColor(instance->kComponentNormal());
    setDisableUnGrooveColor(instance->kComponentNormal());
    setNormalUnGrooveBorderColor(QBrush(QColor(0,0,0,0)));
    setDisableUnGrooveBorderColor(QBrush(QColor(0,0,0,0)));

    setNormalTransparentUnGrooveColor(instance->kComponentAlphaNormal());
    setDisableTransparentUnGrooveColor(instance->kComponentAlphaNormal());
    setNormalTransparentUnGrooveBorderColor(QBrush(QColor(0,0,0,0)));
    setDisableTransparentUnGrooveBorderColor(QBrush(QColor(0,0,0,0)));


    setNormalHandleColor(instance->kFontWhite());
    setHoverHandleColor(instance->kFontWhite());
    setClickHandleColor(instance->kFontWhite());
    setDisableHandleColor(instance->kFontWhite());
    setNormalHandleBorderColor(instance->kBrandNormal());
    setHoverHandleBorderColor(instance->kBrandNormal());
    setClickHandleBorderColor(instance->kBrandNormal());
    setDisableHandleBorderColor(instance->kBrandNormal());
    setTouchHandleBaseColor(instance->baseActive());
    setHandleBorderWidth(3);
    setPadding(0);

    emit parametryChanged();

}

UKUISlider* UKUISlider::qmlAttachedProperties(QObject *parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUISlider(p);
}


int UKUISlider::normalWidth() const
{
    return m_normalWidth;
}

void UKUISlider::setNormalWidth(int newNormalWidth)
{
    if (m_normalWidth == newNormalWidth)
        return;
    m_normalWidth = newNormalWidth;
    emit normalWidthChanged();
}

int UKUISlider::normalHeight() const
{
    return m_normalHeight;
}

void UKUISlider::setNormalHeight(int newNormalHeight)
{
    if (m_normalHeight == newNormalHeight)
        return;
    m_normalHeight = newNormalHeight;
    emit normalHeightChanged();
}

int UKUISlider::grooveHeight() const
{
    return m_grooveHeight;
}

void UKUISlider::setGrooveHeight(int newGrooveHeight)
{
    if (m_grooveHeight == newGrooveHeight)
        return;
    m_grooveHeight = newGrooveHeight;
    emit grooveHeightChanged();
}

int UKUISlider::handleHeight() const
{
    return m_handleHeight;
}

void UKUISlider::setHandleHeight(int newHandleHeight)
{
    if (m_handleHeight == newHandleHeight)
        return;
    m_handleHeight = newHandleHeight;
    emit handleHeightChanged();
}

int UKUISlider::focusBorderWidth() const
{
    return m_focusBorderWidth;
}

void UKUISlider::setFocusBorderWidth(int newFocusBorderWidth)
{
    if (m_focusBorderWidth == newFocusBorderWidth)
        return;
    m_focusBorderWidth = newFocusBorderWidth;
    emit focusBorderWidthChanged();
}

const QBrush &UKUISlider::focusBorderColor() const
{
    return m_focusBorderColor;
}

void UKUISlider::setFocusBorderColor(const QBrush &newFocusBorderColor)
{
    if (m_focusBorderColor == newFocusBorderColor)
        return;
    m_focusBorderColor = newFocusBorderColor;
    emit focusBorderColorChanged();

}

const QBrush &UKUISlider::normalGrooveColor() const
{
    return m_normalGrooveColor;
}

void UKUISlider::setNormalGrooveColor(const QBrush &newNormalGrooveColor)
{
    if (m_normalGrooveColor == newNormalGrooveColor)
        return;
    m_normalGrooveColor = newNormalGrooveColor;
    emit normalGrooveColorChanged();
}

const QBrush &UKUISlider::disableGrooveColor() const
{
    return m_disableGrooveColor;
}

void UKUISlider::setDisableGrooveColor(const QBrush &newDisableGrooveColor)
{
    if (m_disableGrooveColor == newDisableGrooveColor)
        return;
    m_disableGrooveColor = newDisableGrooveColor;
    emit disableGrooveColorChanged();
}

const QBrush &UKUISlider::normalHandleColor() const
{
    return m_normalHandleColor;
}

void UKUISlider::setNormalHandleColor(const QBrush &newNormalHandleColor)
{
    if (m_normalHandleColor == newNormalHandleColor)
        return;
    m_normalHandleColor = newNormalHandleColor;
    emit normalHandleColorChanged();
}

const QBrush &UKUISlider::disableHandleColor() const
{
    return m_disableHandleColor;
}

void UKUISlider::setDisableHandleColor(const QBrush &newDisableHandleColor)
{
    if (m_disableHandleColor == newDisableHandleColor)
        return;
    m_disableHandleColor = newDisableHandleColor;
    emit DisableHandleColorChanged();
}

const QBrush &UKUISlider::normalGrooveBorderColor() const
{
    return m_normalGrooveBorderColor;
}

void UKUISlider::setNormalGrooveBorderColor(const QBrush &newNormalGrooveBorderColor)
{
    if (m_normalGrooveBorderColor == newNormalGrooveBorderColor)
        return;
    m_normalGrooveBorderColor = newNormalGrooveBorderColor;
    emit normalGrooveBorderColorChanged();
}

const QBrush &UKUISlider::disableGrooveBorderColor() const
{
    return m_disableGrooveBorderColor;
}

void UKUISlider::setDisableGrooveBorderColor(const QBrush &newDisableGrooveBorderColor)
{
    if (m_disableGrooveBorderColor == newDisableGrooveBorderColor)
        return;
    m_disableGrooveBorderColor = newDisableGrooveBorderColor;
    emit disableGrooveBorderColorChanged();
}

int UKUISlider::grooveBorderWidth() const
{
    return m_grooveBorderWidth;
}

void UKUISlider::setGrooveBorderWidth(int newGrooveBorderWidth)
{
    if (m_grooveBorderWidth == newGrooveBorderWidth)
        return;
    m_grooveBorderWidth = newGrooveBorderWidth;
    emit grooveBorderWidthChanged();
}

const QBrush &UKUISlider::normalHandleBorderColor() const
{
    return m_normalHandleBorderColor;
}

void UKUISlider::setNormalHandleBorderColor(const QBrush &newNormalHandleBorderColor)
{
    if (m_normalHandleBorderColor == newNormalHandleBorderColor)
        return;
    m_normalHandleBorderColor = newNormalHandleBorderColor;
    emit normalHandleBorderColorChanged();
}

const QBrush &UKUISlider::disableHandleBorderColor() const
{
    return m_disableHandleBorderColor;
}

void UKUISlider::setDisableHandleBorderColor(const QBrush &newDisableHandleBorderColor)
{
    if (m_disableHandleBorderColor == newDisableHandleBorderColor)
        return;
    m_disableHandleBorderColor = newDisableHandleBorderColor;
    emit disableHandleBorderColorChanged();
}

int UKUISlider::handleBorderWidth() const
{
    return m_handleBorderWidth;
}

void UKUISlider::setHandleBorderWidth(int newHandleBorderWidth)
{
    if (m_handleBorderWidth == newHandleBorderWidth)
        return;
    m_handleBorderWidth = newHandleBorderWidth;
    emit handleBorderWidthChanged();
}

const QBrush &UKUISlider::normalUnGrooveColor() const
{
    return m_normalUnGrooveColor;
}

void UKUISlider::setNormalUnGrooveColor(const QBrush &newNormalUnGrooveColor)
{
    if (m_normalUnGrooveColor == newNormalUnGrooveColor)
        return;
    m_normalUnGrooveColor = newNormalUnGrooveColor;
    emit normalUnGrooveColorChanged();
}

const QBrush &UKUISlider::disableUnGrooveColor() const
{
    return m_disableUnGrooveColor;
}

void UKUISlider::setDisableUnGrooveColor(const QBrush &newDisableUnGrooveColor)
{
    if (m_disableUnGrooveColor == newDisableUnGrooveColor)
        return;
    m_disableUnGrooveColor = newDisableUnGrooveColor;
    emit disableUnGrooveColorChanged();
}

const QBrush &UKUISlider::normalUnGrooveBorderColor() const
{
    return m_normalUnGrooveBorderColor;
}

void UKUISlider::setNormalUnGrooveBorderColor(const QBrush &newNormalUnGrooveBorderColor)
{
    if (m_normalUnGrooveBorderColor == newNormalUnGrooveBorderColor)
        return;
    m_normalUnGrooveBorderColor = newNormalUnGrooveBorderColor;
    emit normalUnGrooveBorderColorChanged();
}

const QBrush &UKUISlider::disableUnGrooveBorderColor() const
{
    return m_disableUnGrooveBorderColor;
}

void UKUISlider::setDisableUnGrooveBorderColor(const QBrush &newDisableUnGrooveBorderColor)
{
    if (m_disableUnGrooveBorderColor == newDisableUnGrooveBorderColor)
        return;
    m_disableUnGrooveBorderColor = newDisableUnGrooveBorderColor;
    emit disableUnGrooveBorderColorChanged();
}

const QBrush &UKUISlider::hoverHandleColor() const
{
    return m_hoverHandleColor;
}

void UKUISlider::setHoverHandleColor(const QBrush &newHoverHandleColor)
{
    if (m_hoverHandleColor == newHoverHandleColor)
        return;
    m_hoverHandleColor = newHoverHandleColor;
    emit hoverHandleColorChanged();
}

const QBrush &UKUISlider::clickHandleColor() const
{
    return m_clickHandleColor;
}

void UKUISlider::setClickHandleColor(const QBrush &newClickHandleColor)
{
    if (m_clickHandleColor == newClickHandleColor)
        return;
    m_clickHandleColor = newClickHandleColor;
    emit clickHandleColorChanged();
}

const QBrush &UKUISlider::hoverHandleBorderColor() const
{
    return m_hoverHandleBorderColor;
}

void UKUISlider::setHoverHandleBorderColor(const QBrush &newHoverHandleBorderColor)
{
    if (m_hoverHandleBorderColor == newHoverHandleBorderColor)
        return;
    m_hoverHandleBorderColor = newHoverHandleBorderColor;
    emit hoverHandleBorderColorChanged();
}

const QBrush &UKUISlider::clickHandleBorderColor() const
{
    return m_clickHandleBorderColor;
}

void UKUISlider::setClickHandleBorderColor(const QBrush &newClickHandleBorderColor)
{
    if (m_clickHandleBorderColor == newClickHandleBorderColor)
        return;
    m_clickHandleBorderColor = newClickHandleBorderColor;
    emit clickHandleBorderColorChanged();
}

int UKUISlider::padding() const
{
    return m_padding;
}

void UKUISlider::setPadding(int newPadding)
{
    if (m_padding == newPadding)
        return;
    m_padding = newPadding;
    emit paddingChanged();
}

int UKUISlider::touchNormalWidth() const
{
    return m_touchNormalWidth;
}

void UKUISlider::setTouchNormalWidth(int newTouchNormalWidth)
{
    if (m_touchNormalWidth == newTouchNormalWidth)
        return;
    m_touchNormalWidth = newTouchNormalWidth;
    emit touchNormalWidthChanged();
}

int UKUISlider::touchNormalHeight() const
{
    return m_touchNormalHeight;
}

void UKUISlider::setTouchNormalHeight(int newTouchNormalHeight)
{
    if (m_touchNormalHeight == newTouchNormalHeight)
        return;
    m_touchNormalHeight = newTouchNormalHeight;
    emit touchNormalHeightChanged();
}

int UKUISlider::touchGrooveHeight() const
{
    return m_touchGrooveHeight;
}

void UKUISlider::setTouchGrooveHeight(int newTouchGrooveHeight)
{
    if (m_touchGrooveHeight == newTouchGrooveHeight)
        return;
    m_touchGrooveHeight = newTouchGrooveHeight;
    emit touchGrooveHeightChanged();
}

int UKUISlider::touchHandleHeight() const
{
    return m_touchHandleHeight;
}

void UKUISlider::settouchHandleHeight(int newTouchHandleHeight)
{
    if (m_touchHandleHeight == newTouchHandleHeight)
        return;
    m_touchHandleHeight = newTouchHandleHeight;
    emit touchHandleHeightChanged();
}

int UKUISlider::touchHandleWidth() const
{
    return m_touchHandleWidth;
}

void UKUISlider::setTouchHandleWidth(int newTouchHandleWidth)
{
    if (m_touchHandleWidth == newTouchHandleWidth)
        return;
    m_touchHandleWidth = newTouchHandleWidth;
    emit touchHandleWidthChanged();
}

const QBrush &UKUISlider::touchHandleColor() const
{
    return m_touchHandleColor;
}

void UKUISlider::setTouchHandleColor(const QBrush &newTouchHandleColor)
{
    if (m_touchHandleColor == newTouchHandleColor)
        return;
    m_touchHandleColor = newTouchHandleColor;
    emit touchHandleColorChanged();
}

const QBrush &UKUISlider::normalTransparentUnGrooveColor() const
{
    return m_normalTransparentUnGrooveColor;
}

void UKUISlider::setNormalTransparentUnGrooveColor(const QBrush &newNormalTransparentUnGrooveColor)
{
    if (m_normalTransparentUnGrooveColor == newNormalTransparentUnGrooveColor)
        return;
    m_normalTransparentUnGrooveColor = newNormalTransparentUnGrooveColor;
    emit normalTransparentUnGrooveColorChanged();
}

const QBrush &UKUISlider::disableTransparentUnGrooveColor() const
{
    return m_disableTransparentUnGrooveColor;
}

void UKUISlider::setDisableTransparentUnGrooveColor(const QBrush &newDisableTransparentUnGrooveColor)
{
    if (m_disableTransparentUnGrooveColor == newDisableTransparentUnGrooveColor)
        return;
    m_disableTransparentUnGrooveColor = newDisableTransparentUnGrooveColor;
    emit disableTransparentUnGrooveColorChanged();
}

const QBrush &UKUISlider::normalTransparentUnGrooveBorderColor() const
{
    return m_normalTransparentUnGrooveBorderColor;
}

void UKUISlider::setNormalTransparentUnGrooveBorderColor(const QBrush &newNormalTransparentUnGrooveBorderColor)
{
    if (m_normalTransparentUnGrooveBorderColor == newNormalTransparentUnGrooveBorderColor)
        return;
    m_normalTransparentUnGrooveBorderColor = newNormalTransparentUnGrooveBorderColor;
    emit normalTransparentUnGrooveBorderColorChanged();
}

const QBrush &UKUISlider::disableTransparentUnGrooveBorderColor() const
{
    return m_disableTransparentUnGrooveBorderColor;
}

void UKUISlider::setDisableTransparentUnGrooveBorderColor(const QBrush &newDisableTransparentUnGrooveBorderColor)
{
    if (m_disableTransparentUnGrooveBorderColor == newDisableTransparentUnGrooveBorderColor)
        return;
    m_disableTransparentUnGrooveBorderColor = newDisableTransparentUnGrooveBorderColor;
    emit disableTransparentUnGrooveBorderColorChanged();
}

int UKUISlider::handleHoverHeight() const
{
    return m_handleHoverHeight;
}

void UKUISlider::setHandleHoverHeight(int newHandleHoverHeight)
{
    if (m_handleHoverHeight == newHandleHoverHeight)
        return;
    m_handleHoverHeight = newHandleHoverHeight;
    emit handleHoverHeightChanged();
}

const QBrush &UKUISlider::touchHandleBaseColor() const
{
    return m_touchHandleBaseColor;
}

void UKUISlider::setTouchHandleBaseColor(const QBrush &newTouchHandleBaseColor)
{
    if (m_touchHandleBaseColor == newTouchHandleBaseColor)
        return;
    m_touchHandleBaseColor = newTouchHandleBaseColor;
    emit touchHandleBaseColorChanged();
}
