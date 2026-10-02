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

#ifndef UKUITABBAR_H
#define UKUITABBAR_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QtQml/qqml.h>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class UKUITabBar : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QBrush normalColor READ normalColor WRITE setNormalColor NOTIFY normalColorChanged)

public:
    explicit UKUITabBar(QQuickItem *parent = nullptr);
    ~UKUITabBar();

    void initParam(UKUIGlobalDTConfig::GlobalDTConfig* instance);

    static UKUITabBar* qmlAttachedProperties(QObject* parent);

    const QBrush &normalColor() const;
    void setNormalColor(const QBrush &newNormalColor);

signals:
    void normalColorChanged();

    void parametryChanged();

private:
    Q_INVOKABLE QBrush m_normalColor = QBrush(QColor("#FFFFFF"));

    UKUIGlobalDTConfig::GlobalDTConfig* m_instance = nullptr;

};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUITabBar, QML_HAS_ATTACHED_PROPERTIES)

#endif // UKUITABBAR_H
