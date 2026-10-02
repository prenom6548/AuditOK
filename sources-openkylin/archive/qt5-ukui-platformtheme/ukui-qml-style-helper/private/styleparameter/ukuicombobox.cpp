#include <QApplication>
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

#include "ukuicombobox.h"
#include "qdebug.h"
using namespace UKUIQQC2Style;

UKUIComboBox::UKUIComboBox(QQuickItem *parent) : QQuickItem(parent)
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

UKUIComboBox::~UKUIComboBox()
{

}

UKUIComboBox* UKUIComboBox::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIComboBox(p);
}

void UKUIComboBox::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setLeftRightPadding(8);
    setRadius(instance->kradiusNormal());
    setBorderWidth(instance->normalline());
    setFocusBorderWidth(instance->focusline());
    setNormalWidth(240);
    setNormalHeight(36);
    setUpDownMargin(6);
    setLeftRightPadding(6);
    setNormalBC(instance->kComponentNormal());
    setHoveredBC(instance->kComponentHover());
    setClickedBC(instance->kComponentClick());
    setDisableBC(instance->kComponentDisable());
    setNormalBorderColor(instance->kLineComponentNormal());
    setHoveredBorderColor(instance->kLineComponentHover());
    setClickBorderColor(instance->kLineComponentClick());
    setDisableBorderColor(instance->kLineComponentDisable());
    setFocusBorderColor(instance->kBrandFocus());
    setNormalTextColor(instance->buttonTextActive());
    setDisableTextColor(instance->buttonTextDisable());

    setNormalTransparentBC(instance->kComponentAlphaNormal());
    setClickedTransparentBC(instance->kComponentAlphaClick());
    setHoveredTransparentBC(instance->kComponentAlphaHover());
    setDisableTransparentBC(instance->kComponentAlphaDisable());
    setNormalTransparentBorderColor(instance->kLineComponentNormal());
    setClickedTransparentBorderColor(instance->kLineComponentNormal());
    setHoveredTransparentBorderColor(instance->kLineComponentNormal());
    setDisableTransparentBorderColor(instance->kLineComponentDisable());
    emit parametryChanged();

}

double UKUIComboBox::radius() const
{
    return m_radius;
}

void UKUIComboBox::setRadius(double newRadius)
{
    if (qFuzzyCompare(m_radius, newRadius))
        return;
    m_radius = newRadius;
    emit radiusChanged();
}

double UKUIComboBox::leftRightPadding() const
{
    return m_leftRightPadding;
}

void UKUIComboBox::setLeftRightPadding(double newLeftRightPadding)
{
    if (qFuzzyCompare(m_leftRightPadding, newLeftRightPadding))
        return;
    m_leftRightPadding = newLeftRightPadding;
    emit leftRightPaddingChanged();
}

double UKUIComboBox::upDownMargin() const
{
    return m_upDownMargin;
}

void UKUIComboBox::setUpDownMargin(double newUpDownMargin)
{
    if (qFuzzyCompare(m_upDownMargin, newUpDownMargin))
        return;
    m_upDownMargin = newUpDownMargin;
    emit upDownMarginChanged();
}

double UKUIComboBox::space() const
{
    return m_space;
}

void UKUIComboBox::setSpace(double newSpace)
{
    if (qFuzzyCompare(m_space, newSpace))
        return;
    m_space = newSpace;
    emit spaceChange();
}

int UKUIComboBox::normalWidth() const
{
    return m_normalWidth;
}

void UKUIComboBox::setNormalWidth(int newNormalWidth)
{
    if (m_normalWidth == newNormalWidth)
        return;
    m_normalWidth = newNormalWidth;
    emit normalWidthChanged();
}

int UKUIComboBox::normalHeight() const
{
    return m_normalHeight;
}

void UKUIComboBox::setNormalHeight(int newNormalHeight)
{
    if (m_normalHeight == newNormalHeight)
        return;
    m_normalHeight = newNormalHeight;
    emit normalHeightChanged();
}

QBrush UKUIComboBox::normalBC() const
{
    return m_normalBC;
}

void UKUIComboBox::setNormalBC(QBrush newNormalBC)
{
    if (m_normalBC !=newNormalBC){
    m_normalBC = newNormalBC;
    emit normalBCChanged();
    }
}

QBrush UKUIComboBox::clickedBC() const
{
    return m_clickedBC;
}

void UKUIComboBox::setClickedBC(QBrush newClickedBC)
{
    if (m_clickedBC != newClickedBC){
    m_clickedBC = newClickedBC;
    emit clickedBCChanged();
    }
}

QBrush UKUIComboBox::hoveredBC() const
{
    return m_hoveredBC;
}

void UKUIComboBox::setHoveredBC(QBrush newHoveredBC)
{
    if (m_hoveredBC != newHoveredBC){
    m_hoveredBC = newHoveredBC;
    emit hoveredBCChanged();
    }
}

QBrush UKUIComboBox::disableBC() const
{
    return m_disableBC;
}

void UKUIComboBox::setDisableBC(QBrush newDisableBC)
{
    if (m_disableBC != newDisableBC){
    m_disableBC = newDisableBC;
    emit disableBCChanged();
    }
}

QBrush UKUIComboBox::disableTextColor() const
{
    return m_disableTextColor;
}

void UKUIComboBox::setDisableTextColor(QBrush newDisableTextColor)
{
    if (m_disableTextColor != newDisableTextColor){
    m_disableTextColor = newDisableTextColor;
    emit disableTextColorChanged();
    }
}

QBrush UKUIComboBox::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUIComboBox::setNormalTextColor(QBrush newNormalTextColor)
{
    if (m_normalTextColor != newNormalTextColor){
    m_normalTextColor = newNormalTextColor;
    emit normalTextColorChanged();
    }
}

int UKUIComboBox::borderWidth() const
{
    return m_borderWidth;
}

void UKUIComboBox::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

int UKUIComboBox::focusBorderWidth() const
{
    return m_focusBorderWidth;
}

void UKUIComboBox::setFocusBorderWidth(int newFocusBorderWidth)
{
    if (m_focusBorderWidth == newFocusBorderWidth)
        return;
    m_focusBorderWidth = newFocusBorderWidth;
    emit focusBorderWidthChanged();
}

QBrush UKUIComboBox::normalBorderColor() const
{
    return m_normalBorderColor;
}

void UKUIComboBox::setNormalBorderColor(QBrush newNormalBorderColor)
{
    if (m_normalBorderColor != newNormalBorderColor){
    m_normalBorderColor = newNormalBorderColor;
    emit normalBorderColorChanged();
    }
}

QBrush UKUIComboBox::clickBorderColor() const
{
    return m_clickBorderColor;
}

void UKUIComboBox::setClickBorderColor(QBrush newClickBorderColor)
{
    if (m_clickBorderColor != newClickBorderColor){
    m_clickBorderColor = newClickBorderColor;
    emit clickBorderColorChanged();
    }
}

QBrush UKUIComboBox::disableBorderColor() const
{
    return m_disableBorderColor;
}

void UKUIComboBox::setDisableBorderColor(QBrush newDisableBorderColor)
{
    if (m_disableBorderColor != newDisableBorderColor){
    m_disableBorderColor = newDisableBorderColor;
    emit disableBorderColorChanged();
    }
}

QBrush UKUIComboBox::focusBorderColor() const
{
    return m_focusBorderColor;
}

void UKUIComboBox::setFocusBorderColor(QBrush newFocusBorderColor)
{
    if (m_focusBorderColor != newFocusBorderColor){
    m_focusBorderColor = newFocusBorderColor;
    emit focusBorderColorChanged();
    }
}

const QBrush &UKUIComboBox::normalTransparentBC() const
{
    return m_normalTransparentBC;
}

void UKUIComboBox::setNormalTransparentBC(const QBrush &newNormalTransparentBC)
{
    if (m_normalTransparentBC == newNormalTransparentBC)
        return;
    m_normalTransparentBC = newNormalTransparentBC;
    emit normalTransparentBCChanged();
}

const QBrush &UKUIComboBox::clickedTransparentBC() const
{
    return m_clickedTransparentBC;
}

void UKUIComboBox::setClickedTransparentBC(const QBrush &newClickedTransparentBC)
{
    if (m_clickedTransparentBC == newClickedTransparentBC)
        return;
    m_clickedTransparentBC = newClickedTransparentBC;
    emit clickedTransparentBCChanged();
}

const QBrush &UKUIComboBox::hoveredTransparentBC() const
{
    return m_hoveredTransparentBC;
}

void UKUIComboBox::setHoveredTransparentBC(const QBrush &newHoveredTransparentBC)
{
    if (m_hoveredTransparentBC == newHoveredTransparentBC)
        return;
    m_hoveredTransparentBC = newHoveredTransparentBC;
    emit hoveredTransparentBCChanged();
}

const QBrush &UKUIComboBox::disableTransparentBC() const
{
    return m_disableTransparentBC;
}

void UKUIComboBox::setDisableTransparentBC(const QBrush &newDisableTransparentBC)
{
    if (m_disableTransparentBC == newDisableTransparentBC)
        return;
    m_disableTransparentBC = newDisableTransparentBC;
    emit disableTransparentBCChanged();
}

const QBrush &UKUIComboBox::hoveredBorderColor() const
{
    return m_hoveredBorderColor;
}

void UKUIComboBox::setHoveredBorderColor(const QBrush &newHoveredBorderColor)
{
    if (m_hoveredBorderColor == newHoveredBorderColor)
        return;
    m_hoveredBorderColor = newHoveredBorderColor;
    emit hoveredBorderColorChanged();
}

const QBrush &UKUIComboBox::normalTransparentBorderColor() const
{
    return m_normalTransparentBorderColor;
}

void UKUIComboBox::setNormalTransparentBorderColor(const QBrush &newNormalTransparentBorderColor)
{
    if (m_normalTransparentBorderColor == newNormalTransparentBorderColor)
        return;
    m_normalTransparentBorderColor = newNormalTransparentBorderColor;
    emit normalTransparentBorderColorChanged();
}

const QBrush &UKUIComboBox::clickedTransparentBorderColor() const
{
    return m_clickedTransparentBorderColor;
}

void UKUIComboBox::setClickedTransparentBorderColor(const QBrush &newClickedTransparentBorderColor)
{
    if (m_clickedTransparentBorderColor == newClickedTransparentBorderColor)
        return;
    m_clickedTransparentBorderColor = newClickedTransparentBorderColor;
    emit clickedTransparentBorderColorChanged();
}

const QBrush &UKUIComboBox::hoveredTransparentBorderColor() const
{
    return m_hoveredTransparentBorderColor;
}

void UKUIComboBox::setHoveredTransparentBorderColor(const QBrush &newHoveredTransparentBorderColor)
{
    if (m_hoveredTransparentBorderColor == newHoveredTransparentBorderColor)
        return;
    m_hoveredTransparentBorderColor = newHoveredTransparentBorderColor;
    emit hoveredTransparentBorderColorChanged();
}

const QBrush &UKUIComboBox::disableTransparentBorderColor() const
{
    return m_disableTransparentBorderColor;
}

void UKUIComboBox::setDisableTransparentBorderColor(const QBrush &newDisableTransparentBorderColor)
{
    if (m_disableTransparentBorderColor == newDisableTransparentBorderColor)
        return;
    m_disableTransparentBorderColor = newDisableTransparentBorderColor;
    emit disableTransparentBorderColorChanged();
}
