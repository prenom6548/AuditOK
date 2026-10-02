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

#include "ukuispinbox.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUISpinBox::UKUISpinBox(QQuickItem *parent) : QQuickItem(parent)
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

UKUISpinBox::~UKUISpinBox()
{

}

void UKUISpinBox::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalColor(instance->kComponentNormal());
    setHoverColor(instance->kComponentHover());
    setClickColor(instance->kComponentInput());
    setDisableColor(instance->kComponentDisable());
    setNormalTextColor(instance->buttonTextActive());
    setDisableTextColor(instance->buttonTextDisable());
    setNormalBorderColor(instance->kLineComponentNormal());
    setHoverBorderColor(instance->kLineComponentHover());
    setClickBorderColor(instance->kBrandNormal());
    setDisableBorderColor(instance->kLineComponentDisable());
    setFocusBorderColor(instance->kBrandFocus());
    setFocusColor(instance->kContainGeneralNormal());

    setBtnNormalColor(QBrush(QColor(0,0,0,0)));
    setBtnHoverColor(instance->kComponentHover());
    setBtnClickColor(instance->kComponentClick());
    setBtnDisableColor(QBrush(QColor(0,0,0,0)));
//    setBtnNormalBorderColor(instance->kLineComponentNormal());
//    setBtnHoverBorderColor(instance->kLineComponentHover());
//    setBtnClickBorderColor(instance->kLineComponentClick());
//    setBtnDisableBorderColor(instance->kLineComponentDisable());

    setNormalTransparentBC(instance->kComponentAlphaNormal());
    setClickedTransparentBC(instance->kComponentInputAlpha());
    setHoveredTransparentBC(instance->kComponentAlphaHover());
    setDisableTransparentBC(instance->kComponentAlphaDisable());
    setNormalTransparentBorderColor(instance->kLineComponentNormal());
    setClickedTransparentBorderColor(instance->kBrandNormal());
    setHoveredTransparentBorderColor(instance->kLineComponentHover());
    setDisableTransparentBorderColor(instance->kLineComponentDisable());

    setBtnNormalTransparentColor(QBrush(QColor(0,0,0,0)));
    setBtnHoverTransparentColor(instance->kComponentAlphaHover());
    setBtnClickTransparentColor(instance->kComponentAlphaClick());
    setBtnDisableTransparentColor(QBrush(QColor(0,0,0,0)));
//    setBtnNormalTransparentBorderColor(instance->kLineComponentNormal());
//    setBtnHoverTransparentBorderColor(instance->kLineComponentHover());
//    setBtnClickTransparentBorderColor(instance->kLineComponentClick());
//    setBtnDisableTransparentBorderColor(instance->kLineComponentDisable());

    setNormalWidth(160);
    setNormalHeight(36);
    setBtnNormalWidth(36);
    setBtnNormalHeight(18);
    setRadius(instance->kradiusNormal());
    setNormalBorderWidth(instance->normalline());
    setFocusBorderWidth(instance->focusline());
    setPadding(8);
    setBtnBorderWidth(instance->normalline());

    emit parametryChanged();

}

UKUISpinBox* UKUISpinBox::qmlAttachedProperties(QObject *parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUISpinBox(p);
}

const QBrush &UKUISpinBox::normalColor() const
{
    return m_normalColor;
}

void UKUISpinBox::setNormalColor(const QBrush &newNormalColor)
{
    if (m_normalColor == newNormalColor)
        return;
    m_normalColor = newNormalColor;
    emit normalColorChanged();
}

const QBrush &UKUISpinBox::disableColor() const
{
    return m_disableColor;
}

void UKUISpinBox::setDisableColor(const QBrush &newDisableColor)
{
    if (m_disableColor == newDisableColor)
        return;
    m_disableColor = newDisableColor;
    emit disableColorChanged();
}

const QBrush &UKUISpinBox::hoverColor() const
{
    return m_hoverColor;
}

void UKUISpinBox::setHoverColor(const QBrush &newHoverColor)
{
    if (m_hoverColor == newHoverColor)
        return;
    m_hoverColor = newHoverColor;
    emit hoverColorChanged();
}

const QBrush &UKUISpinBox::clickColor() const
{
    return m_clickColor;
}

void UKUISpinBox::setClickColor(const QBrush &newClickColor)
{
    if (m_clickColor == newClickColor)
        return;
    m_clickColor = newClickColor;
    emit clickColorChanged();
}

const QBrush &UKUISpinBox::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUISpinBox::setNormalTextColor(const QBrush &newNormalTextColor)
{
    if (m_normalTextColor == newNormalTextColor)
        return;
    m_normalTextColor = newNormalTextColor;
    emit normalTextColorChanged();
}

const QBrush &UKUISpinBox::disableTextColor() const
{
    return m_disableTextColor;
}

void UKUISpinBox::setDisableTextColor(const QBrush &newDisableTextColor)
{
    if (m_disableTextColor == newDisableTextColor)
        return;
    m_disableTextColor = newDisableTextColor;
    emit disableTextColorChanged();
}

int UKUISpinBox::normalWidth() const
{
    return m_normalWidth;
}

void UKUISpinBox::setNormalWidth(int newNormalWidth)
{
    if (m_normalWidth == newNormalWidth)
        return;
    m_normalWidth = newNormalWidth;
    emit normalWidthChanged();
}

int UKUISpinBox::normalHeight() const
{
    return m_normalHeight;
}

void UKUISpinBox::setNormalHeight(int newNormalHeight)
{
    if (m_normalHeight == newNormalHeight)
        return;
    m_normalHeight = newNormalHeight;
    emit normalHeightChanged();
}

int UKUISpinBox::radius() const
{
    return m_radius;
}

void UKUISpinBox::setRadius(int newRadius)
{
    if (m_radius == newRadius)
        return;
    m_radius = newRadius;
    emit radiusChanged();
}

int UKUISpinBox::btnNormalWidth() const
{
    return m_btnNormalWidth;
}

void UKUISpinBox::setBtnNormalWidth(int newBtnNormalWidth)
{
    if (m_btnNormalWidth == newBtnNormalWidth)
        return;
    m_btnNormalWidth = newBtnNormalWidth;
    emit btnNormalWidthChanged();
}

int UKUISpinBox::btnNormalHeight() const
{
    return m_btnNormalHeight;
}

void UKUISpinBox::setBtnNormalHeight(int newBtnNormalHeight)
{
    if (m_btnNormalHeight == newBtnNormalHeight)
        return;
    m_btnNormalHeight = newBtnNormalHeight;
    emit btnNormalHeightChanged();
}

const QBrush &UKUISpinBox::normalBorderColor() const
{
    return m_normalBorderColor;
}

void UKUISpinBox::setNormalBorderColor(const QBrush &newNormalBorderColor)
{
    if (m_normalBorderColor == newNormalBorderColor)
        return;
    m_normalBorderColor = newNormalBorderColor;
    emit normalBorderColorChanged();
}

const QBrush &UKUISpinBox::hoverBorderColor() const
{
    return m_hoverBorderColor;
}

void UKUISpinBox::setHoverBorderColor(const QBrush &newHoverBorderColor)
{
    if (m_hoverBorderColor == newHoverBorderColor)
        return;
    m_hoverBorderColor = newHoverBorderColor;
    emit hoverColorBorderChanged();
}

const QBrush &UKUISpinBox::clickBorderColor() const
{
    return m_clickBorderColor;
}

void UKUISpinBox::setClickBorderColor(const QBrush &newClickBorderColor)
{
    if (m_clickBorderColor == newClickBorderColor)
        return;
    m_clickBorderColor = newClickBorderColor;
    emit clickColorBorderChanged();
}

const QBrush &UKUISpinBox::disableBorderColor() const
{
    return m_disableBorderColor;
}

void UKUISpinBox::setDisableBorderColor(const QBrush &newDisableBorderColor)
{
    if (m_disableBorderColor == newDisableBorderColor)
        return;
    m_disableBorderColor = newDisableBorderColor;
    emit disableBorderColorChanged();
}

const QBrush &UKUISpinBox::focusBorderColor() const
{
    return m_focusBorderColor;
}

void UKUISpinBox::setFocusBorderColor(const QBrush &newFocusBorderColor)
{
    if (m_focusBorderColor == newFocusBorderColor)
        return;
    m_focusBorderColor = newFocusBorderColor;
    emit focusBorderColorChanged();
}

int UKUISpinBox::focusBorderWidth() const
{
    return m_focusBorderWidth;
}

void UKUISpinBox::setFocusBorderWidth(int newFocusBorderWidth)
{
    if (m_focusBorderWidth == newFocusBorderWidth)
        return;
    m_focusBorderWidth = newFocusBorderWidth;
    emit focusBorderWidthChanged();
}

int UKUISpinBox::normalBorderWidth() const
{
    return m_normalBorderWidth;
}

void UKUISpinBox::setNormalBorderWidth(int newNormalBorderWidth)
{
    if (m_normalBorderWidth == newNormalBorderWidth)
        return;
    m_normalBorderWidth = newNormalBorderWidth;
    emit normalBorderWidthChanged();
}

const QBrush &UKUISpinBox::focusColor() const
{
    return m_focusColor;
}

void UKUISpinBox::setFocusColor(const QBrush &newFocusColor)
{
    if (m_focusColor == newFocusColor)
        return;
    m_focusColor = newFocusColor;
    emit focusColorChanged();
}

int UKUISpinBox::padding() const
{
    return m_padding;
}

void UKUISpinBox::setPadding(int newPadding)
{
    if (m_padding == newPadding)
        return;
    m_padding = newPadding;
    emit paddingChanged();
}

const QBrush &UKUISpinBox::btnNormalColor() const
{
    return m_btnNormalColor;
}

void UKUISpinBox::setBtnNormalColor(const QBrush &newBtnNormalColor)
{
    if (m_btnNormalColor == newBtnNormalColor)
        return;
    m_btnNormalColor = newBtnNormalColor;
    emit btnNormalColorChanged();
}

const QBrush &UKUISpinBox::btnHoverColor() const
{
    return m_btnHoverColor;
}

void UKUISpinBox::setBtnHoverColor(const QBrush &newBtnHoverColor)
{
    if (m_btnHoverColor == newBtnHoverColor)
        return;
    m_btnHoverColor = newBtnHoverColor;
    emit btnHoverColorChanged();
}

const QBrush &UKUISpinBox::btnClickColor() const
{
    return m_btnClickColor;
}

void UKUISpinBox::setBtnClickColor(const QBrush &newBtnClickColor)
{
    if (m_btnClickColor == newBtnClickColor)
        return;
    m_btnClickColor = newBtnClickColor;
    emit btnClickColorChanged();
}

const QBrush &UKUISpinBox::btnDisableColor() const
{
    return m_btnDisableColor;
}

void UKUISpinBox::setBtnDisableColor(const QBrush &newBtnDisableColor)
{
    if (m_btnDisableColor == newBtnDisableColor)
        return;
    m_btnDisableColor = newBtnDisableColor;
    emit btnDisableColorChanged();
}

const QBrush &UKUISpinBox::btnFocusColor() const
{
    return m_btnFocusColor;
}

void UKUISpinBox::setBtnFocusColor(const QBrush &newBtnFocusColor)
{
    if (m_btnFocusColor == newBtnFocusColor)
        return;
    m_btnFocusColor = newBtnFocusColor;
    emit btnFocusColorChanged();
}

const QBrush &UKUISpinBox::btnNormalBorderColor() const
{
    return m_btnNormalBorderColor;
}

void UKUISpinBox::setBtnNormalBorderColor(const QBrush &newBtnNormalBorderColor)
{
    if (m_btnNormalBorderColor == newBtnNormalBorderColor)
        return;
    m_btnNormalBorderColor = newBtnNormalBorderColor;
    emit btnNormalBorderColorChanged();
}

const QBrush &UKUISpinBox::btnHoverBorderColor() const
{
    return m_btnHoverBorderColor;
}

void UKUISpinBox::setBtnHoverBorderColor(const QBrush &newBtnHoverBorderColor)
{
    if (m_btnHoverBorderColor == newBtnHoverBorderColor)
        return;
    m_btnHoverBorderColor = newBtnHoverBorderColor;
    emit btnHoverBorderColorChanged();
}

const QBrush &UKUISpinBox::btnClickBorderColor() const
{
    return m_btnClickBorderColor;
}

void UKUISpinBox::setBtnClickBorderColor(const QBrush &newBtnClickBorderColor)
{
    if (m_btnClickBorderColor == newBtnClickBorderColor)
        return;
    m_btnClickBorderColor = newBtnClickBorderColor;
    emit btnClickBorderColorChanged();
}

const QBrush &UKUISpinBox::btnDisableBorderColor() const
{
    return m_btnDisableBorderColor;
}

void UKUISpinBox::setBtnDisableBorderColor(const QBrush &newBtnDisableBorderColor)
{
    if (m_btnDisableBorderColor == newBtnDisableBorderColor)
        return;
    m_btnDisableBorderColor = newBtnDisableBorderColor;
    emit btnDisableBorderColorChanged();
}

const QBrush &UKUISpinBox::btnFocusBorderColor() const
{
    return m_btnFocusBorderColor;
}

void UKUISpinBox::setBtnFocusBorderColor(const QBrush &newBtnFocusBorderColor)
{
    if (m_btnFocusBorderColor == newBtnFocusBorderColor)
        return;
    m_btnFocusBorderColor = newBtnFocusBorderColor;
    emit btnFocusBorderColorChanged();
}

int UKUISpinBox::btnBorderWidth() const
{
    return m_btnBorderWidth;
}

void UKUISpinBox::setBtnBorderWidth(int newBtnBorderWidth)
{
    if (m_btnBorderWidth == newBtnBorderWidth)
        return;
    m_btnBorderWidth = newBtnBorderWidth;
    emit btnBorderWidthChanged();
}

const QBrush &UKUISpinBox::normalTransparentBC() const
{
    return m_normalTransparentBC;
}

void UKUISpinBox::setNormalTransparentBC(const QBrush &newNormalTransparentBC)
{
    if (m_normalTransparentBC == newNormalTransparentBC)
        return;
    m_normalTransparentBC = newNormalTransparentBC;
    emit normalTransparentBCChanged();
}

const QBrush &UKUISpinBox::clickedTransparentBC() const
{
    return m_clickedTransparentBC;
}

void UKUISpinBox::setClickedTransparentBC(const QBrush &newClickedTransparentBC)
{
    if (m_clickedTransparentBC == newClickedTransparentBC)
        return;
    m_clickedTransparentBC = newClickedTransparentBC;
    emit clickedTransparentBCChanged();
}

const QBrush &UKUISpinBox::hoveredTransparentBC() const
{
    return m_hoveredTransparentBC;
}

void UKUISpinBox::setHoveredTransparentBC(const QBrush &newHoveredTransparentBC)
{
    if (m_hoveredTransparentBC == newHoveredTransparentBC)
        return;
    m_hoveredTransparentBC = newHoveredTransparentBC;
    emit hoveredTransparentBCChanged();
}

const QBrush &UKUISpinBox::disableTransparentBC() const
{
    return m_disableTransparentBC;
}

void UKUISpinBox::setDisableTransparentBC(const QBrush &newDisableTransparentBC)
{
    if (m_disableTransparentBC == newDisableTransparentBC)
        return;
    m_disableTransparentBC = newDisableTransparentBC;
    emit disableTransparentBCChanged();
}

const QBrush &UKUISpinBox::normalTransparentBorderColor() const
{
    return m_normalTransparentBorderColor;
}

void UKUISpinBox::setNormalTransparentBorderColor(const QBrush &newNormalTransparentBorderColor)
{
    if (m_normalTransparentBorderColor == newNormalTransparentBorderColor)
        return;
    m_normalTransparentBorderColor = newNormalTransparentBorderColor;
    emit normalTransparentBorderColorChanged();
}

const QBrush &UKUISpinBox::clickedTransparentBorderColor() const
{
    return m_clickedTransparentBorderColor;
}

void UKUISpinBox::setClickedTransparentBorderColor(const QBrush &newClickedTransparentBorderColor)
{
    if (m_clickedTransparentBorderColor == newClickedTransparentBorderColor)
        return;
    m_clickedTransparentBorderColor = newClickedTransparentBorderColor;
    emit clickedTransparentBorderColorChanged();
}

const QBrush &UKUISpinBox::hoveredTransparentBorderColor() const
{
    return m_hoveredTransparentBorderColor;
}

void UKUISpinBox::setHoveredTransparentBorderColor(const QBrush &newHoveredTransparentBorderColor)
{
    if (m_hoveredTransparentBorderColor == newHoveredTransparentBorderColor)
        return;
    m_hoveredTransparentBorderColor = newHoveredTransparentBorderColor;
    emit hoveredTransparentBorderColorChanged();
}

const QBrush &UKUISpinBox::disableTransparentBorderColor() const
{
    return m_disableTransparentBorderColor;
}

void UKUISpinBox::setDisableTransparentBorderColor(const QBrush &newDisableTransparentBorderColor)
{
    if (m_disableTransparentBorderColor == newDisableTransparentBorderColor)
        return;
    m_disableTransparentBorderColor = newDisableTransparentBorderColor;
    emit disableTransparentBorderColorChanged();
}

const QBrush &UKUISpinBox::btnNormalTransparentColor() const
{
    return m_btnNormalTransparentColor;
}

void UKUISpinBox::setBtnNormalTransparentColor(const QBrush &newBtnNormalTransparentColor)
{
    if (m_btnNormalTransparentColor == newBtnNormalTransparentColor)
        return;
    m_btnNormalTransparentColor = newBtnNormalTransparentColor;
    emit btnNormalTransparentColorChanged();
}

const QBrush &UKUISpinBox::btnHoverTransparentColor() const
{
    return m_btnHoverTransparentColor;
}

void UKUISpinBox::setBtnHoverTransparentColor(const QBrush &newBtnHoverTransparentColor)
{
    if (m_btnHoverTransparentColor == newBtnHoverTransparentColor)
        return;
    m_btnHoverTransparentColor = newBtnHoverTransparentColor;
    emit btnHoverTransparentColorChanged();
}

const QBrush &UKUISpinBox::btnClickTransparentColor() const
{
    return m_btnClickTransparentColor;
}

void UKUISpinBox::setBtnClickTransparentColor(const QBrush &newBtnClickTransparentColor)
{
    if (m_btnClickTransparentColor == newBtnClickTransparentColor)
        return;
    m_btnClickTransparentColor = newBtnClickTransparentColor;
    emit btnClickTransparentColorChanged();
}

const QBrush &UKUISpinBox::btnDisableTransparentColor() const
{
    return m_btnDisableTransparentColor;
}

void UKUISpinBox::setBtnDisableTransparentColor(const QBrush &newBtnDisableTransparentColor)
{
    if (m_btnDisableTransparentColor == newBtnDisableTransparentColor)
        return;
    m_btnDisableTransparentColor = newBtnDisableTransparentColor;
    emit btnDisableTransparentColorChanged();
}

const QBrush &UKUISpinBox::btnNormalTransparentBorderColor() const
{
    return m_btnNormalTransparentBorderColor;
}

void UKUISpinBox::setBtnNormalTransparentBorderColor(const QBrush &newBtnNormalTransparentBorderColor)
{
    if (m_btnNormalTransparentBorderColor == newBtnNormalTransparentBorderColor)
        return;
    m_btnNormalTransparentBorderColor = newBtnNormalTransparentBorderColor;
    emit btnNormalTransparentBorderColorChanged();
}

const QBrush &UKUISpinBox::btnHoverTransparentBorderColor() const
{
    return m_btnHoverTransparentBorderColor;
}

void UKUISpinBox::setBtnHoverTransparentBorderColor(const QBrush &newBtnHoverTransparentBorderColor)
{
    if (m_btnHoverTransparentBorderColor == newBtnHoverTransparentBorderColor)
        return;
    m_btnHoverTransparentBorderColor = newBtnHoverTransparentBorderColor;
    emit btnHoverTransparentBorderColorChanged();
}

const QBrush &UKUISpinBox::btnClickTransparentBorderColor() const
{
    return m_btnClickTransparentBorderColor;
}

void UKUISpinBox::setBtnClickTransparentBorderColor(const QBrush &newBtnClickTransparentBorderColor)
{
    if (m_btnClickTransparentBorderColor == newBtnClickTransparentBorderColor)
        return;
    m_btnClickTransparentBorderColor = newBtnClickTransparentBorderColor;
    emit btnClickTransparentBorderColorChanged();
}

const QBrush &UKUISpinBox::btnDisableTransparentBorderColor() const
{
    return m_btnDisableTransparentBorderColor;
}

void UKUISpinBox::setBtnDisableTransparentBorderColor(const QBrush &newBtnDisableTransparentBorderColor)
{
    if (m_btnDisableTransparentBorderColor == newBtnDisableTransparentBorderColor)
        return;
    m_btnDisableTransparentBorderColor = newBtnDisableTransparentBorderColor;
    emit btnDisableTransparentBorderColorChanged();
}
