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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef UKUIHeaderView_H
#define UKUIHeaderView_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUIHeaderView : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QBrush normalTextColor READ normalTextColor WRITE setNormalTextColor NOTIFY normalTextColorChanged)
    Q_PROPERTY(QBrush lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(int normalHeight READ normalHeight WRITE setnormalHeight NOTIFY lineWidthChanged)


public:
    explicit UKUIHeaderView(QQuickItem *parent = nullptr);
    ~UKUIHeaderView();

    static UKUIHeaderView* qmlAttachedProperties(QObject* parent);

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    const QBrush &normalTextColor() const;
    void setNormalTextColor(const QBrush &newNormalTextColor);

    const QBrush &lineColor() const;
    void setLineColor(const QBrush &newLineColor);

    int lineWidth() const;
    void setLineWidth(int newLineWidth);

    int normalHeight() const;
    void setnormalHeight(int newNormalHeight);

signals:

    void parametryChanged();
    
    void normalTextColorChanged();

    void lineColorChanged();

    void lineWidthChanged();

private:

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;
    
    QBrush m_normalTextColor;
    QBrush m_lineColor;
    int m_lineWidth;
    int m_normalHeight;
};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIHeaderView, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUIHeaderView_H
