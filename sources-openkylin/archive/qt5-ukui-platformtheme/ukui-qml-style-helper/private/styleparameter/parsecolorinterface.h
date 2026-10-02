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

#ifndef PARSEDTCOLOR_H
#define PARSEDTCOLOR_H

#include <QObject>
#include "../../../ukui-styles/readconfig.h"
#include <QColor>
#include <QPen>

using namespace UKUIGlobalDTConfig;
namespace UKUIQQC2Style {

struct ColorStruct{
    bool isSolidPattern = true;
    QColor startColor;
    QColor endColor;
};

class ParseColorInterface : public QObject
{
    Q_OBJECT

public:
    explicit ParseColorInterface(QObject *parent = nullptr);

    static ParseColorInterface* qmlAttachedProperties(QObject* parent);

    Q_INVOKABLE bool isSolidPattern(QVariant dtColor);

    Q_INVOKABLE QColor startColor(QVariant dtColor);

    Q_INVOKABLE QColor endColor(QVariant dtColor);

    Q_INVOKABLE QString endString(QString s, QString lasts);

    Q_INVOKABLE QColor getDtColor(QVariant dtColorkey);

};
}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::ParseColorInterface, QML_HAS_ATTACHED_PROPERTIES)
QML_DECLARE_TYPEINFO(UKUIQQC2Style::ColorStruct, QML_HAS_ATTACHED_PROPERTIES)

#endif // PARSEDTCOLOR_H
