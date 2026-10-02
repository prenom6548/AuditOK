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

#include "ukuilabel.h"
#include "qdebug.h"

using namespace UKUIQQC2Style;

UKUILabel::UKUILabel(QQuickItem *parent) : QQuickItem(parent)
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

UKUILabel::~UKUILabel()
{

}

void UKUILabel::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalColor(instance->textActive());
    setDisableColor(instance->textDisable());

    emit parametryChanged();

}

UKUILabel* UKUILabel::qmlAttachedProperties(QObject *parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUILabel(p);
}

const QBrush  &UKUILabel::normalColor() const
{
    return m_normalColor;
}

void UKUILabel::setNormalColor(const QBrush  &newNormalColor)
{
    if (m_normalColor == newNormalColor)
        return;
    m_normalColor = newNormalColor;
    emit normalColorChanged();
}

const QBrush  &UKUILabel::linkColor() const
{
    return m_linkColor;
}

void UKUILabel::setLinkColor(const QBrush  &newLinkColor)
{
    if (m_linkColor == newLinkColor)
        return;
    m_linkColor = newLinkColor;
    emit linkColorChanged();
}


const QBrush  &UKUILabel::disableColor() const
{
    return m_disableColor;
}

void UKUILabel::setDisableColor(const QBrush  &newDisableColor)
{
    if (m_disableColor == newDisableColor)
        return;
    m_disableColor = newDisableColor;
    emit disableColorChanged();
}
