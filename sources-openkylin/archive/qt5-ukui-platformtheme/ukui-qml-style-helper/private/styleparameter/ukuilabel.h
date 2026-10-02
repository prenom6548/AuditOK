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

#ifndef UKUILABEL_H
#define UKUILABEL_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUILabel : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QBrush  normalColor READ normalColor WRITE setNormalColor NOTIFY normalColorChanged)
    Q_PROPERTY(QBrush  disableColor READ disableColor WRITE setDisableColor NOTIFY disableColorChanged)
    Q_PROPERTY(QBrush  linkColor READ linkColor WRITE setLinkColor NOTIFY linkColorChanged)

public:
    explicit UKUILabel(QQuickItem *parent = nullptr);
    ~UKUILabel();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUILabel* qmlAttachedProperties(QObject* parent);

    const QBrush  &normalColor() const;
    void setNormalColor(const QBrush  &newNormalColor);
    const QBrush  &linkColor() const;
    void setLinkColor(const QBrush  &newLinkColor);


    const QBrush  &disableColor() const;
    void setDisableColor(const QBrush  &newDisableColor);

signals:
    void normalColorChanged();

    void linkColorChanged();

    void disableColorChanged();

    void parametryChanged();

private:
    Q_INVOKABLE QBrush  m_normalColor = QBrush(QColor ::fromRgbF(0, 0, 0, 0.85));

    Q_INVOKABLE QBrush  m_linkColor = QBrush(Qt::blue);
    Q_INVOKABLE QBrush  m_disableColor;

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUILabel, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUILABEL_H
