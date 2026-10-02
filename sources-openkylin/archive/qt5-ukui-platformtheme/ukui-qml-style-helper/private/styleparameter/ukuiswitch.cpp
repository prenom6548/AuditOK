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

#include "ukuiswitch.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUISwitch::UKUISwitch(QQuickItem *parent) : QQuickItem(parent)
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

UKUISwitch::~UKUISwitch()
{

}

void UKUISwitch::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalColor(instance->kComponentNormal());
    setHoverColor(instance->kComponentHover());
    setClickColor(instance->kComponentClick());
    setDisableColor(instance->kComponentDisable());
    setNormalBorderColor(instance->kLineComponentNormal());
    setHoverBorderColor(instance->kLineComponentHover());
    setClickBorderColor(instance->kLineComponentClick());
    setDisableBorderColor(instance->kLineComponentDisable());

    setCheckedNormalColor(instance->kBrandNormal());
    setCheckedHoverColor(instance->kBrandHover());
    setCheckedClickColor(instance->kBrandClick());
    setCheckedDisableColor(instance->kBrandDisable());
    setCheckedNormalBorderColor(instance->kLineComponentNormal());
    setCheckedHoverBorderColor(instance->kLineComponentHover());
    setCheckedClickBorderColor(instance->kLineComponentClick());
    setCheckedDisableBorderColor(instance->kLineComponentDisable());

    setNormalIndicatorColor(instance->kFontWhite());
    setDisableIndicatorColor(instance->kFontWhiteDisable());

    setNormalTransparentColor(instance->kComponentAlphaNormal());
    setHoverTransparentColor(instance->kComponentAlphaHover());
    setClickTransparentColor(instance->kComponentAlphaClick());
    setDisableTransparentColor(instance->kComponentAlphaDisable());
    setNormalTransparentBorderColor(instance->kLineComponentNormal());
    setHoverTransparentBorderColor(instance->kLineComponentHover());
    setClickTransparentBorderColor(instance->kLineComponentClick());
    setDisableTransparentBorderColor(instance->kLineComponentDisable());

    setRecWidth(44);
    setRecHeight(22);
    setIndicatorWidth(16);
    setPadding(4);
    setBorderWidth(instance->normalline());
    setIndicatorLeftRightPadding(4);
    emit parametryChanged();

}

UKUISwitch* UKUISwitch::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUISwitch(p);
}

const QBrush &UKUISwitch::normalColor() const
{
    return m_normalColor;
}

void UKUISwitch::setNormalColor(const QBrush &newNormalColor)
{
    if (m_normalColor == newNormalColor)
        return;
    m_normalColor = newNormalColor;
    emit normalColorChanged();
}


int UKUISwitch::padding() const
{
    return m_padding;
}

void UKUISwitch::setPadding(int newPadding)
{
    if (m_padding == newPadding)
        return;
    m_padding = newPadding;
    emit paddingChanged();
}

const QBrush &UKUISwitch::hoverColor() const
{
    return m_hoverColor;
}

void UKUISwitch::setHoverColor(const QBrush &newHoverColor)
{
    if (m_hoverColor == newHoverColor)
        return;
    m_hoverColor = newHoverColor;
    emit hoverColorChanged();
}

const QBrush &UKUISwitch::clickColor() const
{
    return m_clickColor;
}

void UKUISwitch::setClickColor(const QBrush &newClickColor)
{
    if (m_clickColor == newClickColor)
        return;
    m_clickColor = newClickColor;
    emit clickColorChanged();
}

const QBrush &UKUISwitch::disableColor() const
{
    return m_disableColor;
}

void UKUISwitch::setDisableColor(const QBrush &newDisableColor)
{
    if (m_disableColor == newDisableColor)
        return;
    m_disableColor = newDisableColor;
    emit disableColorChanged();
}

const QBrush &UKUISwitch::checkedNormalColor() const
{
    return m_checkedNormalColor;
}

void UKUISwitch::setCheckedNormalColor(const QBrush &newCheckedNormalColor)
{
    if (m_checkedNormalColor == newCheckedNormalColor)
        return;
    m_checkedNormalColor = newCheckedNormalColor;
    emit checkedNormalColorChanged();
}

const QBrush &UKUISwitch::checkedHoverColor() const
{
    return m_checkedHoverColor;
}

void UKUISwitch::setCheckedHoverColor(const QBrush &newCheckedHoverColor)
{
    if (m_checkedHoverColor == newCheckedHoverColor)
        return;
    m_checkedHoverColor = newCheckedHoverColor;
    emit checkedHoverColorChanged();
}

const QBrush &UKUISwitch::checkedClickColor() const
{
    return m_checkedClickColor;
}

void UKUISwitch::setCheckedClickColor(const QBrush &newCheckedClickColor)
{
    if (m_checkedClickColor == newCheckedClickColor)
        return;
    m_checkedClickColor = newCheckedClickColor;
    emit checkedClickColorChanged();
}

const QBrush &UKUISwitch::checkedDisableColor() const
{
    return m_checkedDisableColor;
}

void UKUISwitch::setCheckedDisableColor(const QBrush &newCheckedDisableColor)
{
    if (m_checkedDisableColor == newCheckedDisableColor)
        return;
    m_checkedDisableColor = newCheckedDisableColor;
    emit checkedDisableColorChanged();
}

const QBrush &UKUISwitch::normalBorderColor() const
{
    return m_normalBorderColor;
}

void UKUISwitch::setNormalBorderColor(const QBrush &newNormalBorderColor)
{
    if (m_normalBorderColor == newNormalBorderColor)
        return;
    m_normalBorderColor = newNormalBorderColor;
    emit normalBorderColorChanged();
}

const QBrush &UKUISwitch::hoverBorderColor() const
{
    return m_hoverBorderColor;
}

void UKUISwitch::setHoverBorderColor(const QBrush &newHoverBorderColor)
{
    if (m_hoverBorderColor == newHoverBorderColor)
        return;
    m_hoverBorderColor = newHoverBorderColor;
    emit hoverBorderColorChanged();
}

const QBrush &UKUISwitch::clickBorderColor() const
{
    return m_clickBorderColor;
}

void UKUISwitch::setClickBorderColor(const QBrush &newClickBorderColor)
{
    if (m_clickBorderColor == newClickBorderColor)
        return;
    m_clickBorderColor = newClickBorderColor;
    emit clickBorderColorChanged();
}

const QBrush &UKUISwitch::disableBorderColor() const
{
    return m_disableBorderColor;
}

void UKUISwitch::setDisableBorderColor(const QBrush &newDisableBorderColor)
{
    if (m_disableBorderColor == newDisableBorderColor)
        return;
    m_disableBorderColor = newDisableBorderColor;
    emit disableBorderColorChanged();
}

const QBrush &UKUISwitch::checkedNormalBorderColor() const
{
    return m_checkedNormalBorderColor;
}

void UKUISwitch::setCheckedNormalBorderColor(const QBrush &newCheckedNormalBorderColor)
{
    if (m_checkedNormalBorderColor == newCheckedNormalBorderColor)
        return;
    m_checkedNormalBorderColor = newCheckedNormalBorderColor;
    emit checkedNormalBorderColorChanged();
}

const QBrush &UKUISwitch::checkedHoverBorderColor() const
{
    return m_checkedHoverBorderColor;
}

void UKUISwitch::setCheckedHoverBorderColor(const QBrush &newCheckedHoverBorderColor)
{
    if (m_checkedHoverBorderColor == newCheckedHoverBorderColor)
        return;
    m_checkedHoverBorderColor = newCheckedHoverBorderColor;
    emit checkedHoverBorderColorChanged();
}

const QBrush &UKUISwitch::checkedClickBorderColor() const
{
    return m_checkedClickBorderColor;
}

void UKUISwitch::setCheckedClickBorderColor(const QBrush &newCheckedClickBorderColor)
{
    if (m_checkedClickBorderColor == newCheckedClickBorderColor)
        return;
    m_checkedClickBorderColor = newCheckedClickBorderColor;
    emit checkedClickBorderColorChanged();
}

const QBrush &UKUISwitch::checkedDisableBorderColor() const
{
    return m_checkedDisableBorderColor;
}

void UKUISwitch::setCheckedDisableBorderColor(const QBrush &newCheckedDisableBorderColor)
{
    if (m_checkedDisableBorderColor == newCheckedDisableBorderColor)
        return;
    m_checkedDisableBorderColor = newCheckedDisableBorderColor;
    emit checkedDisableBorderColorChanged();
}

int UKUISwitch::borderWidth() const
{
    return m_borderWidth;
}

void UKUISwitch::setBorderWidth(int newBorderWidth)
{
    if (m_borderWidth == newBorderWidth)
        return;
    m_borderWidth = newBorderWidth;
    emit borderWidthChanged();
}

const QBrush &UKUISwitch::normalIndicatorColor() const
{
    return m_normalIndicatorColor;
}

void UKUISwitch::setNormalIndicatorColor(const QBrush &newNormalIndicatorColor)
{
    if (m_normalIndicatorColor == newNormalIndicatorColor)
        return;
    m_normalIndicatorColor = newNormalIndicatorColor;
    emit normalIndicatorColorChanged();
}

const QBrush &UKUISwitch::disableIndicatorColor() const
{
    return m_disableIndicatorColor;
}

void UKUISwitch::setDisableIndicatorColor(const QBrush &newDisableIndicatorColor)
{
    if (m_disableIndicatorColor == newDisableIndicatorColor)
        return;
    m_disableIndicatorColor = newDisableIndicatorColor;
    emit disableIndicatorColorChanged();
}

int UKUISwitch::indicatorWidth() const
{
    return m_indicatorWidth;
}

void UKUISwitch::setIndicatorWidth(int newIndicatorWidth)
{
    if (m_indicatorWidth == newIndicatorWidth)
        return;
    m_indicatorWidth = newIndicatorWidth;
    emit indicatorWidthChanged();
}

int UKUISwitch::recWidth() const
{
    return m_recWidth;
}

void UKUISwitch::setRecWidth(int newRecWidth)
{
    if (m_recWidth == newRecWidth)
        return;
    m_recWidth = newRecWidth;
    emit recWidthHChanged();
}

int UKUISwitch::recHeight() const
{
    return m_recHeight;
}

void UKUISwitch::setRecHeight(int newRecHeight)
{
    if (m_recHeight == newRecHeight)
        return;
    m_recHeight = newRecHeight;
    emit recHeightChanged();
}

int UKUISwitch::indicatorLeftRightPadding() const
{
    return m_indicatorLeftRightPadding;
}

void UKUISwitch::setIndicatorLeftRightPadding(int newIndicatorLeftRightPadding)
{
    if (m_indicatorLeftRightPadding == newIndicatorLeftRightPadding)
        return;
    m_indicatorLeftRightPadding = newIndicatorLeftRightPadding;
    emit indicatorLeftRightPaddingChanged();
}

const QBrush &UKUISwitch::normalTransparentColor() const
{
    return m_normalTransparentColor;
}

void UKUISwitch::setNormalTransparentColor(const QBrush &newNormalTransparentColor)
{
    if (m_normalTransparentColor == newNormalTransparentColor)
        return;
    m_normalTransparentColor = newNormalTransparentColor;
    emit normalTransparentColorChanged();
}

const QBrush &UKUISwitch::hoverTransparentColor() const
{
    return m_hoverTransparentColor;
}

void UKUISwitch::setHoverTransparentColor(const QBrush &newHoverTransparentColor)
{
    if (m_hoverTransparentColor == newHoverTransparentColor)
        return;
    m_hoverTransparentColor = newHoverTransparentColor;
    emit hoverTransparentColorChanged();
}

const QBrush &UKUISwitch::clickTransparentColor() const
{
    return m_clickTransparentColor;
}

void UKUISwitch::setClickTransparentColor(const QBrush &newClickTransparentColor)
{
    if (m_clickTransparentColor == newClickTransparentColor)
        return;
    m_clickTransparentColor = newClickTransparentColor;
    emit clickTransparentColorChanged();
}

const QBrush &UKUISwitch::disableTransparentColor() const
{
    return m_disableTransparentColor;
}

void UKUISwitch::setDisableTransparentColor(const QBrush &newDisableTransparentColor)
{
    if (m_disableTransparentColor == newDisableTransparentColor)
        return;
    m_disableTransparentColor = newDisableTransparentColor;
    emit disableTransparentColorChanged();
}

const QBrush &UKUISwitch::normalTransparentBorderColor() const
{
    return m_normalTransparentBorderColor;
}

void UKUISwitch::setNormalTransparentBorderColor(const QBrush &newNormalTransparentBorderColor)
{
    if (m_normalTransparentBorderColor == newNormalTransparentBorderColor)
        return;
    m_normalTransparentBorderColor = newNormalTransparentBorderColor;
    emit normalTransparentBorderColorChanged();
}

const QBrush &UKUISwitch::hoverTransparentBorderColor() const
{
    return m_hoverTransparentBorderColor;
}

void UKUISwitch::setHoverTransparentBorderColor(const QBrush &newHoverTransparentBorderColor)
{
    if (m_hoverTransparentBorderColor == newHoverTransparentBorderColor)
        return;
    m_hoverTransparentBorderColor = newHoverTransparentBorderColor;
    emit hoverTransparentBorderColorChanged();
}

const QBrush &UKUISwitch::clickTransparentBorderColor() const
{
    return m_clickTransparentBorderColor;
}

void UKUISwitch::setClickTransparentBorderColor(const QBrush &newClickTransparentBorderColor)
{
    if (m_clickTransparentBorderColor == newClickTransparentBorderColor)
        return;
    m_clickTransparentBorderColor = newClickTransparentBorderColor;
    emit clickTransparentBorderColorChanged();
}

const QBrush &UKUISwitch::disableTransparentBorderColor() const
{
    return m_disableTransparentBorderColor;
}

void UKUISwitch::setDisableTransparentBorderColor(const QBrush &newDisableTransparentBorderColor)
{
    if (m_disableTransparentBorderColor == newDisableTransparentBorderColor)
        return;
    m_disableTransparentBorderColor = newDisableTransparentBorderColor;
    emit disableTransparentBorderColorChanged();
}
