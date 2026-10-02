/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#ifndef TOOLUTILS_H
#define TOOLUTILS_H

#include <QObject>
#include <QLabel>

namespace KylinRubbishClear
{

class ToolUtils : public QObject
{
    Q_OBJECT
public:
    explicit ToolUtils(QObject *parent = nullptr);
    static void setToolTipAfterChangeFont(QLabel *label, QString str);
};

} // namespace KylinRubbishClear


#endif // TOOLUTILS_H
