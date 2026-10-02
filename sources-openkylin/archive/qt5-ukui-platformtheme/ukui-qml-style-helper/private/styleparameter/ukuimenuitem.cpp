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

#include "ukuimenuitem.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUIMenuItem::UKUIMenuItem(QQuickItem *parent) : QQuickItem(parent)
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

UKUIMenuItem::~UKUIMenuItem()
{

}

void UKUIMenuItem::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setBorderWidth(1);
    setNormalBC(QBrush(QColor(0,0,0,0)));
    setHoveredBC(instance->kComponentHover());
    setClickedBC(instance->kComponentClick());
    setDisableBC(QBrush(QColor(0,0,0,0)));
    setNormalBorderColor(QBrush(QColor(0,0,0,0)));
    setHoverBorderColor(instance->kLineComponentHover());
    setClickBorderColor(instance->kLineComponentClick());
    setDisableBorderColor(QBrush(QColor(0,0,0,0)));
    setNormalTextColor(instance->kFontPrimary());
    setDisableTextColor(instance->kFontPrimaryDisable());

    setNormalTransparentBC(QBrush(QColor(0,0,0,0)));
    setHoveredTransparentBC(instance->kComponentAlphaHover());
    setClickedTransparentBC(instance->kComponentAlphaClick());
    setDisableTransparentBC(QBrush(QColor(0,0,0,0)));
    setNormalTransparentBorderColor(QBrush(QColor(0,0,0,0)));
    setHoverTransparentBorderColor(instance->kLineComponentHover());
    setClickTransparentBorderColor(instance->kLineComponentClick());
    setDisableTransparentBorderColor(QBrush(QColor(0,0,0,0)));

    setNormalCheckedBC(instance->kComponentSelectedNormal());
    setNormalBorderCheckedColor(instance->kLineComponentNormal());
    setHoveredCheckedBC(instance->kComponentSelectedHover());
    setHoverBorderCheckedColor(instance->kLineComponentHover());
    setClickedCheckedBC(instance->kComponentSelectedClick());
    setClickBorderCheckedColor(instance->kLineComponentClick());
    setDisableCheckedBC(instance->kComponentSelectedDisable());
    setDisableBorderCheckedColor(instance->kLineComponentDisable());

    setNormalCheckedTransparentBC(instance->kComponentSelectedAlphaNormal());
    setNormalBorderCheckedTransparentColor(instance->kLineComponentNormal());
    setHoveredCheckedTransparentBC(instance->kComponentSelectedAlphaHover());
    setHoverBorderCheckedTransparentColor(instance->kLineComponentHover());
    setClickedCheckedTransparentBC(instance->kComponentSelectedAlphaClick());
    setClickBorderCheckedTransparentColor(instance->kLineComponentClick());
    setDisableCheckedTransparentBC(instance->kComponentSelectedAlphaDisable());
    setDisableBorderCheckedTransparentColor(instance->kLineComponentDisable());

    setNormalHTextColor(instance->kFontWhite());
    setDisableHTextColor(instance->kFontWhiteDisable());
    setNormalCheckedHBC(instance->kBrandNormal());
    setHoveredCheckedHBC(instance->kBrandHover());
    setClickedCheckedHBC(instance->kBrandClick());
    setDisableCheckedHBC(instance->kBrandDisable());
    setNormalCheckedHBorderColor(instance->kLineBrandNormal());
    setHoveredCheckedHBorderColor(instance->kLineBrandHover());
    setClickedCheckedHBorderColor(instance->kLineBrandClick());
    setDisableCheckedHBorderColor(instance->kLineBrandDisable());

    setMenuSeparatorColor(instance->kLineNormal());


    emit parametryChanged();

}

UKUIMenuItem* UKUIMenuItem::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIMenuItem(p);
}

int UKUIMenuItem::leftRightPadding() const
{
    return m_leftRightPadding;
}

void UKUIMenuItem::setLeftRightPadding(int newLeftRightPadding)
{
    if (m_leftRightPadding == newLeftRightPadding)
        return;
    m_leftRightPadding = newLeftRightPadding;
    emit leftRightPaddingChanged();
}

int UKUIMenuItem::topBottomPadding() const
{
    return m_topBottomPadding;
}

void UKUIMenuItem::setTopBottomPadding(int newTopBottomPadding)
{
    if (m_topBottomPadding == newTopBottomPadding)
        return;
    m_topBottomPadding = newTopBottomPadding;
    emit topBottomPaddingChanged();
}

int UKUIMenuItem::imageWidth() const
{
    return m_imageWidth;
}

void UKUIMenuItem::setImageWidth(int newImageWidth)
{
    if (m_imageWidth == newImageWidth)
        return;
    m_imageWidth = newImageWidth;
    emit imageWidthChanged();
}

int UKUIMenuItem::imageSpace() const
{
    return m_imageSpace;
}

void UKUIMenuItem::setImageSpace(int newImageSpace)
{
    if (m_imageSpace == newImageSpace)
        return;
    m_imageSpace = newImageSpace;
    emit imageSpaceChanged();
}

int UKUIMenuItem::normalHeight() const
{
    return m_normalHeight;
}

void UKUIMenuItem::setNormalHeight(int newNormalHeight)
{
    if (m_normalHeight == newNormalHeight)
        return;
    m_normalHeight = newNormalHeight;
    emit normalHeightChanged();
}

int UKUIMenuItem::radius() const
{
    return m_radius;
}

void UKUIMenuItem::setRadius(int newRadius)
{
    if (m_radius == newRadius)
        return;
    m_radius = newRadius;
    emit RadiusChanged();
}


const QBrush &UKUIMenuItem::normalBC() const
{
    return m_normalBC;
}

void UKUIMenuItem::setNormalBC(const QBrush &newNormalBC)
{
    if (m_normalBC == newNormalBC)
        return;
    m_normalBC = newNormalBC;
    emit normalBCChanged();
}

const QBrush &UKUIMenuItem::clickedBC() const
{
    return m_clickedBC;
}

void UKUIMenuItem::setClickedBC(const QBrush &newClickedBC)
{
    if (m_clickedBC == newClickedBC)
        return;
    m_clickedBC = newClickedBC;
    emit clickedBCChanged();
}

const QBrush &UKUIMenuItem::hoveredBC() const
{
    return m_hoveredBC;
}

void UKUIMenuItem::setHoveredBC(const QBrush &newHoveredBC)
{
    if (m_hoveredBC == newHoveredBC)
        return;
    m_hoveredBC = newHoveredBC;
    emit hoveredBCChanged();
}

const QBrush &UKUIMenuItem::disableBC() const
{
    return m_disableBC;
}

void UKUIMenuItem::setDisableBC(const QBrush &newDisableBC)
{
    if (m_disableBC == newDisableBC)
        return;
    m_disableBC = newDisableBC;
    emit disableBCChanged();
}

const QBrush &UKUIMenuItem::disableTextColor() const
{
    return m_disableTextColor;
}

void UKUIMenuItem::setDisableTextColor(const QBrush &newDisableTextColor)
{
    if (m_disableTextColor == newDisableTextColor)
        return;
    m_disableTextColor = newDisableTextColor;
    emit disableTextColorChanged();
}

const QBrush &UKUIMenuItem::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUIMenuItem::setNormalTextColor(const QBrush &newNormalTextColor)
{
    if (m_normalTextColor == newNormalTextColor)
        return;
    m_normalTextColor = newNormalTextColor;
    emit disableTextColorChanged();
}

int UKUIMenuItem::borderWidth() const
{
    return m_borderWidth;
}

void UKUIMenuItem::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

const QBrush &UKUIMenuItem::normalBorderColor() const
{
    return m_normalBorderColor;
}

void UKUIMenuItem::setNormalBorderColor(const QBrush &newNormalBorderColor)
{
    if (m_normalBorderColor == newNormalBorderColor)
        return;
    m_normalBorderColor = newNormalBorderColor;
    emit normalBorderColorChanged();
}

const QBrush &UKUIMenuItem::hoverBorderColor() const
{
    return m_hoverBorderColor;
}

void UKUIMenuItem::setHoverBorderColor(const QBrush &newHoverBorderColor)
{
    if (m_hoverBorderColor == newHoverBorderColor)
        return;
    m_hoverBorderColor = newHoverBorderColor;
    emit hoverBorderColorChanged();
}

const QBrush &UKUIMenuItem::clickBorderColor() const
{
    return m_clickBorderColor;
}

void UKUIMenuItem::setClickBorderColor(const QBrush &newClickBorderColor)
{
    if (m_clickBorderColor == newClickBorderColor)
        return;
    m_clickBorderColor = newClickBorderColor;
    emit clickBorderColorChanged();
}

const QBrush &UKUIMenuItem::disableBorderColor() const
{
    return m_disableBorderColor;
}

void UKUIMenuItem::setDisableBorderColor(const QBrush &newDisableBorderColor)
{
    if (m_disableBorderColor == newDisableBorderColor)
        return;
    m_disableBorderColor = newDisableBorderColor;
    emit disableBorderColorChanged();
}

const QBrush &UKUIMenuItem::normalTransparentBC() const
{
    return m_normalTransparentBC;
}

void UKUIMenuItem::setNormalTransparentBC(const QBrush &newNormalTransparentBC)
{
    if (m_normalTransparentBC == newNormalTransparentBC)
        return;
    m_normalTransparentBC = newNormalTransparentBC;
    emit normalTransparentBCChanged();
}

const QBrush &UKUIMenuItem::clickedTransparentBC() const
{
    return m_clickedTransparentBC;
}

void UKUIMenuItem::setClickedTransparentBC(const QBrush &newClickedTransparentBC)
{
    if (m_clickedTransparentBC == newClickedTransparentBC)
        return;
    m_clickedTransparentBC = newClickedTransparentBC;
    emit clickedTransparentBCChanged();
}

const QBrush &UKUIMenuItem::hoveredTransparentBC() const
{
    return m_hoveredTransparentBC;
}

void UKUIMenuItem::setHoveredTransparentBC(const QBrush &newHoveredTransparentBC)
{
    if (m_hoveredTransparentBC == newHoveredTransparentBC)
        return;
    m_hoveredTransparentBC = newHoveredTransparentBC;
    emit hoveredTransparentBCChanged();
}

const QBrush &UKUIMenuItem::disableTransparentBC() const
{
    return m_disableTransparentBC;
}

void UKUIMenuItem::setDisableTransparentBC(const QBrush &newDisableTransparentBC)
{
    if (m_disableTransparentBC == newDisableTransparentBC)
        return;
    m_disableTransparentBC = newDisableTransparentBC;
    emit disableTransparentBCChanged();
}

const QBrush &UKUIMenuItem::normalTransparentBorderColor() const
{
    return m_normalTransparentBorderColor;
}

void UKUIMenuItem::setNormalTransparentBorderColor(const QBrush &newNormalTransparentBorderColor)
{
    if (m_normalTransparentBorderColor == newNormalTransparentBorderColor)
        return;
    m_normalTransparentBorderColor = newNormalTransparentBorderColor;
    emit normalTransparentBorderColorChanged();
}

const QBrush &UKUIMenuItem::hoverTransparentBorderColor() const
{
    return m_hoverTransparentBorderColor;
}

void UKUIMenuItem::setHoverTransparentBorderColor(const QBrush &newHoverTransparentBorderColor)
{
    if (m_hoverTransparentBorderColor == newHoverTransparentBorderColor)
        return;
    m_hoverTransparentBorderColor = newHoverTransparentBorderColor;
    emit hoverTransparentBorderColorChanged();
}

const QBrush &UKUIMenuItem::clickTransparentBorderColor() const
{
    return m_clickTransparentBorderColor;
}

void UKUIMenuItem::setClickTransparentBorderColor(const QBrush &newClickTransparentBorderColor)
{
    if (m_clickTransparentBorderColor == newClickTransparentBorderColor)
        return;
    m_clickTransparentBorderColor = newClickTransparentBorderColor;
    emit clickTransparentBorderColorChanged();
}

const QBrush &UKUIMenuItem::disableTransparentBorderColor() const
{
    return m_disableTransparentBorderColor;
}

void UKUIMenuItem::setDisableTransparentBorderColor(const QBrush &newDisableTransparentBorderColor)
{
    if (m_disableTransparentBorderColor == newDisableTransparentBorderColor)
        return;
    m_disableTransparentBorderColor = newDisableTransparentBorderColor;
    emit disableTransparentBorderColorChanged();
}

const QBrush &UKUIMenuItem::normalCheckedBC() const
{
    return m_normalCheckedBC;
}

void UKUIMenuItem::setNormalCheckedBC(const QBrush &newNormalCheckedBC)
{
    if (m_normalCheckedBC == newNormalCheckedBC)
        return;
    m_normalCheckedBC = newNormalCheckedBC;
    emit normalCheckedBCChanged();
}

const QBrush &UKUIMenuItem::clickedCheckedBC() const
{
    return m_clickedCheckedBC;
}

void UKUIMenuItem::setClickedCheckedBC(const QBrush &newClickedCheckedBC)
{
    if (m_clickedCheckedBC == newClickedCheckedBC)
        return;
    m_clickedCheckedBC = newClickedCheckedBC;
    emit clickedCheckedBCChanged();
}

const QBrush &UKUIMenuItem::hoveredCheckedBC() const
{
    return m_hoveredCheckedBC;
}

void UKUIMenuItem::setHoveredCheckedBC(const QBrush &newHoveredCheckedBC)
{
    if (m_hoveredCheckedBC == newHoveredCheckedBC)
        return;
    m_hoveredCheckedBC = newHoveredCheckedBC;
    emit hoveredCheckedBCChanged();
}

const QBrush &UKUIMenuItem::disableCheckedBC() const
{
    return m_disableCheckedBC;
}

void UKUIMenuItem::setDisableCheckedBC(const QBrush &newDisableCheckedBC)
{
    if (m_disableCheckedBC == newDisableCheckedBC)
        return;
    m_disableCheckedBC = newDisableCheckedBC;
    emit disableCheckedBCChanged();
}

const QBrush &UKUIMenuItem::normalBorderCheckedColor() const
{
    return m_normalBorderCheckedColor;
}

void UKUIMenuItem::setNormalBorderCheckedColor(const QBrush &newNormalBorderCheckedColor)
{
    if (m_normalBorderCheckedColor == newNormalBorderCheckedColor)
        return;
    m_normalBorderCheckedColor = newNormalBorderCheckedColor;
    emit normalBorderCheckedColorChanged();
}

const QBrush &UKUIMenuItem::hoverBorderCheckedColor() const
{
    return m_hoverBorderCheckedColor;
}

void UKUIMenuItem::setHoverBorderCheckedColor(const QBrush &newHoverBorderCheckedColor)
{
    if (m_hoverBorderCheckedColor == newHoverBorderCheckedColor)
        return;
    m_hoverBorderCheckedColor = newHoverBorderCheckedColor;
    emit hoverBorderCheckedColorChanged();
}

const QBrush &UKUIMenuItem::clickBorderCheckedColor() const
{
    return m_clickBorderCheckedColor;
}

void UKUIMenuItem::setClickBorderCheckedColor(const QBrush &newClickBorderCheckedColor)
{
    if (m_clickBorderCheckedColor == newClickBorderCheckedColor)
        return;
    m_clickBorderCheckedColor = newClickBorderCheckedColor;
    emit clickBorderCheckedColorChanged();
}

const QBrush &UKUIMenuItem::disableBorderCheckedColor() const
{
    return m_disableBorderCheckedColor;
}

void UKUIMenuItem::setDisableBorderCheckedColor(const QBrush &newDisableBorderCheckedColor)
{
    if (m_disableBorderCheckedColor == newDisableBorderCheckedColor)
        return;
    m_disableBorderCheckedColor = newDisableBorderCheckedColor;
    emit disableBorderCheckedColorChanged();
}

const QBrush &UKUIMenuItem::normalHTextColor() const
{
    return m_normalHTextColor;
}

void UKUIMenuItem::setNormalHTextColor(const QBrush &newNormalHTextColor)
{
    if (m_normalHTextColor == newNormalHTextColor)
        return;
    m_normalHTextColor = newNormalHTextColor;
    emit normalHTextColorChanged();
}

const QBrush &UKUIMenuItem::disableHTextColor() const
{
    return m_disableHTextColor;
}

void UKUIMenuItem::setDisableHTextColor(const QBrush &newDisableHTextColor)
{
    if (m_disableHTextColor == newDisableHTextColor)
        return;
    m_disableHTextColor = newDisableHTextColor;
    emit disableHTextColorChanged();
}

const QBrush &UKUIMenuItem::normalCheckedHBC() const
{
    return m_normalCheckedHBC;
}

void UKUIMenuItem::setNormalCheckedHBC(const QBrush &newNormalCheckedHBC)
{
    if (m_normalCheckedHBC == newNormalCheckedHBC)
        return;
    m_normalCheckedHBC = newNormalCheckedHBC;
    emit normalCheckedHBCChanged();
}

const QBrush &UKUIMenuItem::clickedCheckedHBC() const
{
    return m_clickedCheckedHBC;
}

void UKUIMenuItem::setClickedCheckedHBC(const QBrush &newClickedCheckedHBC)
{
    if (m_clickedCheckedHBC == newClickedCheckedHBC)
        return;
    m_clickedCheckedHBC = newClickedCheckedHBC;
    emit clickedCheckedHBCChanged();
}

const QBrush &UKUIMenuItem::hoveredCheckedHBC() const
{
    return m_hoveredCheckedHBC;
}

void UKUIMenuItem::setHoveredCheckedHBC(const QBrush &newHoveredCheckedHBC)
{
    if (m_hoveredCheckedHBC == newHoveredCheckedHBC)
        return;
    m_hoveredCheckedHBC = newHoveredCheckedHBC;
    emit hoveredCheckedHBCChanged();
}

const QBrush &UKUIMenuItem::disableCheckedHBC() const
{
    return m_disableCheckedHBC;
}

void UKUIMenuItem::setDisableCheckedHBC(const QBrush &newDisableCheckedHBC)
{
    if (m_disableCheckedHBC == newDisableCheckedHBC)
        return;
    m_disableCheckedHBC = newDisableCheckedHBC;
    emit disableCheckedHBCChanged();
}

const QBrush &UKUIMenuItem::normalCheckedHBorderColor() const
{
    return m_normalCheckedHBorderColor;
}

void UKUIMenuItem::setNormalCheckedHBorderColor(const QBrush &newNormalCheckedHBorderColor)
{
    if (m_normalCheckedHBorderColor == newNormalCheckedHBorderColor)
        return;
    m_normalCheckedHBorderColor = newNormalCheckedHBorderColor;
    emit normalCheckedHBorderColorChanged();
}

const QBrush &UKUIMenuItem::clickedCheckedHBorderColor() const
{
    return m_clickedCheckedHBorderColor;
}

void UKUIMenuItem::setClickedCheckedHBorderColor(const QBrush &newClickedCheckedHBorderColor)
{
    if (m_clickedCheckedHBorderColor == newClickedCheckedHBorderColor)
        return;
    m_clickedCheckedHBorderColor = newClickedCheckedHBorderColor;
    emit clickedCheckedHBorderColorChanged();
}

const QBrush &UKUIMenuItem::hoveredCheckedHBorderColor() const
{
    return m_hoveredCheckedHBorderColor;
}

void UKUIMenuItem::setHoveredCheckedHBorderColor(const QBrush &newHoveredCheckedHBorderColor)
{
    if (m_hoveredCheckedHBorderColor == newHoveredCheckedHBorderColor)
        return;
    m_hoveredCheckedHBorderColor = newHoveredCheckedHBorderColor;
    emit hoveredCheckedHBorderColorChanged();
}

const QBrush &UKUIMenuItem::disableCheckedHBorderColor() const
{
    return m_disableCheckedHBorderColor;
}

void UKUIMenuItem::setDisableCheckedHBorderColor(const QBrush &newDisableCheckedHBorderColor)
{
    if (m_disableCheckedHBorderColor == newDisableCheckedHBorderColor)
        return;
    m_disableCheckedHBorderColor = newDisableCheckedHBorderColor;
    emit disableCheckedHBorderColorChanged();
}

const QBrush &UKUIMenuItem::normalCheckedTransparentBC() const
{
    return m_normalCheckedTransparentBC;
}

void UKUIMenuItem::setNormalCheckedTransparentBC(const QBrush &newNormalCheckedTransparentBC)
{
    if (m_normalCheckedTransparentBC == newNormalCheckedTransparentBC)
        return;
    m_normalCheckedTransparentBC = newNormalCheckedTransparentBC;
    emit normalCheckedTransparentBCChanged();
}

const QBrush &UKUIMenuItem::clickedCheckedTransparentBC() const
{
    return m_clickedCheckedTransparentBC;
}

void UKUIMenuItem::setClickedCheckedTransparentBC(const QBrush &newClickedCheckedTransparentBC)
{
    if (m_clickedCheckedTransparentBC == newClickedCheckedTransparentBC)
        return;
    m_clickedCheckedTransparentBC = newClickedCheckedTransparentBC;
    emit clickedCheckedTransparentBCChanged();
}

const QBrush &UKUIMenuItem::hoveredCheckedTransparentBC() const
{
    return m_hoveredCheckedTransparentBC;
}

void UKUIMenuItem::setHoveredCheckedTransparentBC(const QBrush &newHoveredCheckedTransparentBC)
{
    if (m_hoveredCheckedTransparentBC == newHoveredCheckedTransparentBC)
        return;
    m_hoveredCheckedTransparentBC = newHoveredCheckedTransparentBC;
    emit hoveredCheckedTransparentBCChanged();
}

const QBrush &UKUIMenuItem::disableCheckedTransparentBC() const
{
    return m_disableCheckedTransparentBC;
}

void UKUIMenuItem::setDisableCheckedTransparentBC(const QBrush &newDisableCheckedTransparentBC)
{
    if (m_disableCheckedTransparentBC == newDisableCheckedTransparentBC)
        return;
    m_disableCheckedTransparentBC = newDisableCheckedTransparentBC;
    emit disableCheckedTransparentBCChanged();
}

const QBrush &UKUIMenuItem::normalBorderCheckedTransparentColor() const
{
    return m_normalBorderCheckedTransparentColor;
}

void UKUIMenuItem::setNormalBorderCheckedTransparentColor(const QBrush &newNormalBorderCheckedTransparentColor)
{
    if (m_normalBorderCheckedTransparentColor == newNormalBorderCheckedTransparentColor)
        return;
    m_normalBorderCheckedTransparentColor = newNormalBorderCheckedTransparentColor;
    emit normalBorderCheckedColorTransparentChanged();
}

const QBrush &UKUIMenuItem::hoverBorderCheckedTransparentColor() const
{
    return m_hoverBorderCheckedTransparentColor;
}

void UKUIMenuItem::setHoverBorderCheckedTransparentColor(const QBrush &newHoverBorderCheckedTransparentColor)
{
    if (m_hoverBorderCheckedTransparentColor == newHoverBorderCheckedTransparentColor)
        return;
    m_hoverBorderCheckedTransparentColor = newHoverBorderCheckedTransparentColor;
    emit hoverBorderCheckedColorTransparentChanged();
}

const QBrush &UKUIMenuItem::clickBorderCheckedTransparentColor() const
{
    return m_clickBorderCheckedTransparentColor;
}

void UKUIMenuItem::setClickBorderCheckedTransparentColor(const QBrush &newClickBorderCheckedTransparentColor)
{
    if (m_clickBorderCheckedTransparentColor == newClickBorderCheckedTransparentColor)
        return;
    m_clickBorderCheckedTransparentColor = newClickBorderCheckedTransparentColor;
    emit clickBorderCheckedColorTransparentChanged();
}

const QBrush &UKUIMenuItem::disableBorderCheckedTransparentColor() const
{
    return m_disableBorderCheckedTransparentColor;
}

void UKUIMenuItem::setDisableBorderCheckedTransparentColor(const QBrush &newDisableBorderCheckedTransparentColor)
{
    if (m_disableBorderCheckedTransparentColor == newDisableBorderCheckedTransparentColor)
        return;
    m_disableBorderCheckedTransparentColor = newDisableBorderCheckedTransparentColor;
    emit disableBorderCheckedColorTransparentChanged();
}

const QBrush &UKUIMenuItem::menuSeparatorColor() const
{
    return m_menuSeparatorColor;
}

void UKUIMenuItem::setMenuSeparatorColor(const QBrush &newMenuSeparatorColor)
{
    if (m_menuSeparatorColor == newMenuSeparatorColor)
        return;
    m_menuSeparatorColor = newMenuSeparatorColor;
    emit menuSeparatorColorChanged();
}
