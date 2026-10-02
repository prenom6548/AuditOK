/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#ifndef X11KEYBOARD_H
#define X11KEYBOARD_H

#include <QObject>
#include <QMetaEnum>
#include "fakekeyboard.h"

class X11Keyboard : public FakeKeyboard 
{
    Q_OBJECT
public:
    explicit X11Keyboard(QObject *parent = nullptr);
    ~X11Keyboard();
    void addModifier(Modifier::MOD mod);
    void removeModifier(Modifier::MOD mod);
    bool hasModifier(Modifier::MOD mod);
    QList<Modifier::MOD> getAllModifier();
    void clearModifier();

public Q_SLOTS:
    void onKeyPressed(QChar c);
    void onKeyPressed(FuncKey::FUNCKEY key);

private:
    void sendKey(unsigned int keyCode);

private:
    QList<Modifier::MOD> modList;
};

#endif // X11KEYBOARD_H
