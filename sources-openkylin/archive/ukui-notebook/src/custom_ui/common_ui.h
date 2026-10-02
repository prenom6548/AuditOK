/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef COMMON_UI_H
#define COMMON_UI_H

#include <QString>
#include <QWidget>

extern QString dealMessage(const QString msg);

extern void centerTheWindow(QWidget *self, QWidget *parentWindow);

#ifdef TEST_DEBUG
extern void showWindowFrame(QWidget *self);
#endif

#endif // COMMON_UI_H
