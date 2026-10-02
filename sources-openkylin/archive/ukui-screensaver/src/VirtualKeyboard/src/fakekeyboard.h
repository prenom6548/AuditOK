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
#ifndef FAKEKEYBOARD_H
#define FAKEKEYBOARD_H

#include <QObject>
#include <QMetaEnum>

class Modifier : public QObject
{
    Q_OBJECT
public:
    Modifier(){}

    enum MOD{
        UNKNOWN = -1,
        CTRL,
        SHIFT,
        ALT,
        SUPER
    };
    Q_ENUM(MOD)

    static QString getModifierName(int mod)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<Modifier::MOD>();
        const char* modName = metaEnum.valueToKey(mod);
        QString result = QString(modName).toLower();
        return result;
    }
    static MOD getModifier(const QString &modName)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<Modifier::MOD>();
        MOD mod = (MOD)metaEnum.keyToValue(modName.toUpper().toLocal8Bit().data());
        return mod;
    }
};

class FuncKey : public QObject
{
    Q_OBJECT
public:
    FuncKey(){}

    enum FUNCKEY {
        UNKNOWN = -1,
        SPACE = 0,
        BACKSPACE,
        ENTER,
        HOME,
        END,
        PGUP,
        PGDN,
        INSERT,
        DELETE,
        UP,
        DOWN,
        LEFT,
        RIGHT,
        CAPSLOCK
    };
    Q_ENUM(FUNCKEY)
    static QString getKeyName(int key)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<FuncKey::FUNCKEY>();
        const char* keyName = metaEnum.valueToKey(key);
        QString result = QString(keyName).toLower();
        return result;
    }
    static FUNCKEY getKey(const QString &keyName)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<FuncKey::FUNCKEY>();
        FUNCKEY key = (FUNCKEY)metaEnum.keyToValue(keyName.toUpper().toLocal8Bit().data());
        return key;
    }
};

class FakeKeyboard : public QObject
{
    Q_OBJECT
public:
    explicit FakeKeyboard(QObject *parent = nullptr)
        : QObject(parent)
    {

    }

    virtual void addModifier(Modifier::MOD mod) = 0;
    virtual void removeModifier(Modifier::MOD mod) = 0;
    virtual bool hasModifier(Modifier::MOD mod) = 0;
    virtual QList<Modifier::MOD> getAllModifier() = 0;
    virtual void clearModifier() = 0;

public Q_SLOTS:
    virtual void onKeyPressed(QChar c) = 0;
    virtual void onKeyPressed(FuncKey::FUNCKEY key) = 0;
};

#endif // FAKEKEYBOARD_H
