/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UKUIDECORATIONMANAGER_H
#define UKUIDECORATIONMANAGER_H

#include <QWindow>

class UKUIDecorationManager
{
public:
    static UKUIDecorationManager *getInstance();
    bool supportUKUIDecoration();

    bool moveWindow(QWindow *windowHandle);
    bool removeHeaderBar(QWindow *windowHandle);
    bool setCornerRadius(QWindow *windowHandle, int topleft, int topright, int bottomleft, int bottomright);

private:
    UKUIDecorationManager();
};

#endif // UKUIDECORATIONMANAGER_H
