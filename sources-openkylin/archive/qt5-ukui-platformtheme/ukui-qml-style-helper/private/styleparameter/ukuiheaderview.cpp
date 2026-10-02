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

#include "ukuiheaderview.h"
#include "qdebug.h"
using namespace UKUIQQC2Style;

UKUIHeaderView::UKUIHeaderView(QQuickItem *parent) : QQuickItem(parent)
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

UKUIHeaderView::~UKUIHeaderView()
{

}

UKUIHeaderView* UKUIHeaderView::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new UKUIHeaderView(p);
}

void UKUIHeaderView::initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance)
{
    setNormalTextColor(m_instance->kFontPrimary());
    setLineColor(m_instance->kLineInputNormal());
    setLineWidth(1);
    setnormalHeight(36);
    emit parametryChanged();
}


const QBrush &UKUIHeaderView::normalTextColor() const
{
    return m_normalTextColor;
}

void UKUIHeaderView::setNormalTextColor(const QBrush &newNormalTextColor)
{
    if (m_normalTextColor == newNormalTextColor)
        return;
    m_normalTextColor = newNormalTextColor;
    emit normalTextColorChanged();
}

const QBrush &UKUIHeaderView::lineColor() const
{
    return m_lineColor;
}

void UKUIHeaderView::setLineColor(const QBrush &newLineColor)
{
    if (m_lineColor == newLineColor)
        return;
    m_lineColor = newLineColor;
    emit lineColorChanged();
}


int UKUIHeaderView::lineWidth() const
{
    return m_lineWidth;
}

void UKUIHeaderView::setLineWidth(int newLineWidth)
{
    if (m_lineWidth == newLineWidth)
        return;
    m_lineWidth = newLineWidth;
    emit lineWidthChanged();
}

int UKUIHeaderView::normalHeight() const
{
    return m_normalHeight;
}

void UKUIHeaderView::setnormalHeight(int newNormalHeight)
{
    if (m_normalHeight == newNormalHeight)
        return;
    m_normalHeight = newNormalHeight;
    emit lineWidthChanged();
}
