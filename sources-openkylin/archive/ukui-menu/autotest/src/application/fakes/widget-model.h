/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef AUTOTEST_APPLICATION_FAKE_WIDGET_MODEL_H
#define AUTOTEST_APPLICATION_FAKE_WIDGET_MODEL_H

#include <QObject>

namespace UkuiMenu {

class WidgetMetadata
{
    Q_GADGET
public:
    enum Type {
        Widget = 0
    };
    Q_ENUM(Type)
    Q_DECLARE_FLAGS(Types, Type)

    enum Flag {
        Normal = 0x0
    };
    Q_ENUM(Flag)
    Q_DECLARE_FLAGS(Flags, Flag)
};

class WidgetModel : public QObject
{
    Q_OBJECT
public:
    explicit WidgetModel(QObject *parent = nullptr) : QObject(parent) {}
};

} // namespace UkuiMenu

Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiMenu::WidgetMetadata::Types)
Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiMenu::WidgetMetadata::Flags)

#endif
