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

#include "ukuiscrollbar.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUIScrollBar::UKUIScrollBar(QQuickItem *parent) : QQuickItem(parent)
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

UKUIScrollBar::~UKUIScrollBar()
{

}

void UKUIScrollBar::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalColor(instance->kGray12());
    setHoverColor(instance->kGray13());
    setClickColor(instance->kGray14());
    setDisableColor(instance->kGray8());
    setNormalTransparentColor(instance->kGrayAlpha10());
    setHoverTransparentColor(instance->kGrayAlpha13());
    setClickTransparentColor(instance->kGrayAlpha11());
    setDisableTransparentColor(instance->kGrayAlpha5());

    emit parametryChanged();

}

UKUIScrollBar* UKUIScrollBar::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIScrollBar(p);
}

const QBrush &UKUIScrollBar::normalColor() const
{
    return m_normalColor;
}

void UKUIScrollBar::setNormalColor(const QBrush &newNormalColor)
{
    if (m_normalColor == newNormalColor)
        return;
    m_normalColor = newNormalColor;
    emit normalColorChanged();
}


int UKUIScrollBar::padding() const
{
    return m_padding;
}

void UKUIScrollBar::setPadding(int newPadding)
{
    if (m_padding == newPadding)
        return;
    m_padding = newPadding;
    emit paddingChanged();
}

const QBrush &UKUIScrollBar::hoverColor() const
{
    return m_hoverColor;
}

void UKUIScrollBar::setHoverColor(const QBrush &newHoverColor)
{
    if (m_hoverColor == newHoverColor)
        return;
    m_hoverColor = newHoverColor;
    emit hoverColorChanged();
}

const QBrush &UKUIScrollBar::clickColor() const
{
    return m_clickColor;
}

void UKUIScrollBar::setClickColor(const QBrush &newClickColor)
{
    if (m_clickColor == newClickColor)
        return;
    m_clickColor = newClickColor;
    emit clickColorChanged();
}

const QBrush &UKUIScrollBar::disableColor() const
{
    return m_disableColor;
}

void UKUIScrollBar::setDisableColor(const QBrush &newDisableColor)
{
    if (m_disableColor == newDisableColor)
        return;
    m_disableColor = newDisableColor;
    emit disableColorChanged();
}

const QBrush &UKUIScrollBar::normalTransparentColor() const
{
    return m_normalTransparentColor;
}

void UKUIScrollBar::setNormalTransparentColor(const QBrush &newNormalTransparentColor)
{
    if (m_normalTransparentColor == newNormalTransparentColor)
        return;
    m_normalTransparentColor = newNormalTransparentColor;
    emit normalTransparentColorChanged();
}

const QBrush &UKUIScrollBar::hoverTransparentColor() const
{
    return m_hoverTransparentColor;
}

void UKUIScrollBar::setHoverTransparentColor(const QBrush &newHoverTransparentColor)
{
    if (m_hoverTransparentColor == newHoverTransparentColor)
        return;
    m_hoverTransparentColor = newHoverTransparentColor;
    emit hoverTransparentColorChanged();
}

const QBrush &UKUIScrollBar::clickTransparentColor() const
{
    return m_clickTransparentColor;
}

void UKUIScrollBar::setClickTransparentColor(const QBrush &newClickTransparentColor)
{
    if (m_clickTransparentColor == newClickTransparentColor)
        return;
    m_clickTransparentColor = newClickTransparentColor;
    emit clickTransparentColorChanged();
}

const QBrush &UKUIScrollBar::disableTransparentColor() const
{
    return m_disableTransparentColor;
}

void UKUIScrollBar::setDisableTransparentColor(const QBrush &newDisableTransparentColor)
{
    if (m_disableTransparentColor == newDisableTransparentColor)
        return;
    m_disableTransparentColor = newDisableTransparentColor;
    emit disableTransparentColorChanged();
}
