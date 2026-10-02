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
#ifndef APPPARAMETER_H
#define APPPARAMETER_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QPalette>
#include <QQuickItem>
#include "tokenparameter.h"

namespace UKUIQQC2Style {

class APPParameter : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(QPalette palette READ palette WRITE setPalette NOTIFY paletteChanged)
    Q_PROPERTY(int iconWidth READ iconWidth WRITE setIconWidth NOTIFY iconWidthChanged)
    Q_PROPERTY(int space READ space WRITE setSpace NOTIFY spaceChanged)
    Q_PROPERTY(bool isDark READ isDark WRITE setIsDark NOTIFY isDarkChanged)
    Q_PROPERTY(double menuTransparency READ menuTransparency WRITE setMenuTransparency NOTIFY menuTransparencyChanged)
    Q_PROPERTY(QBrush windowColor READ windowColor WRITE setWindowColor NOTIFY windowColorChanged)
    Q_PROPERTY(bool focusEnable READ focusEnable WRITE setFocusEnable NOTIFY focusEnableChanged)
    Q_PROPERTY(QBrush focusBorderColor  READ focusBorderColor WRITE setFocusBorderColor NOTIFY focusBorderColorChanged)
    Q_PROPERTY(int focusBorderWidth  READ focusBorderWidth WRITE setFocusBorderWidth NOTIFY focusBorderWidthChanged)


public:
    explicit APPParameter(QQuickItem *parent = nullptr);
    ~APPParameter();

    static APPParameter* qmlAttachedProperties(QObject* parent);

    const QFont &font() const;
    void setFont(const QFont &newFont);

    const QPalette &palette() const;
    void setPalette(const QPalette &newPalette);

    int iconWidth() const;
    void setIconWidth(int newIconWidth);

    int space() const;
    void setSpace(int newSpace);

    bool isDark() const;
    void setIsDark(bool newIsDark);

    double menuTransparency() const;
    void setMenuTransparency(double newMenuTransparency);

    const QBrush &windowColor() const;
    void setWindowColor(const QBrush &newWindowColor);

    Q_INVOKABLE bool themeHasIcon(QString iconName);

    Q_INVOKABLE bool showToolTipWindow();

    Q_INVOKABLE QPoint posByCursor(int width, int height);

    bool focusEnable() const;

    void setFocusEnable(bool newFocusEnable);

    QBrush focusBorderColor() const;
    void setFocusBorderColor(QBrush newFocusBorderColor);

    int focusBorderWidth() const;
    void setFocusBorderWidth(int newFocusBorderWidth);

public Q_SLOTS:

    void slotChangeStyle(const QString& key);

signals:
    void fontChanged();

    void paletteChanged();

    void iconWidthChanged();

    void spaceChanged();

    void isDarkChanged();

    void parametryChanged();

    void menuTransparencyChanged();

    void windowColorChanged();

    void focusEnableChanged();

    void iconThemeChanged();

    void focusBorderColorChanged();

    void focusBorderWidthChanged();

private:
    QFont m_font;
    QPalette m_palette;
    int m_iconWidth = 16;
    int m_space = 8;
    bool m_isDark = false;
    double m_menuTransparency;
    QBrush m_windowColor;
    bool m_focusEnable = true;
    QBrush m_focusBorderColor;
    int m_focusBorderWidth;
};

inline int APPParameter::iconWidth() const
{
    return m_iconWidth;
}


inline void APPParameter::setIconWidth(int newIconWidth)
{
    if (m_iconWidth == newIconWidth)
        return;
    m_iconWidth = newIconWidth;
    emit iconWidthChanged();
}

inline int APPParameter::space() const
{
    return m_space;
}

inline void APPParameter::setSpace(int newSpace)
{
    if (m_space == newSpace)
        return;
    m_space = newSpace;
    emit spaceChanged();
}

inline bool APPParameter::isDark() const
{
    return m_isDark;
}

inline void APPParameter::setIsDark(bool newIsDark)
{
    if (m_isDark == newIsDark)
        return;
    m_isDark = newIsDark;
    emit isDarkChanged();
}

}
QML_DECLARE_TYPEINFO(UKUIQQC2Style::APPParameter, QML_HAS_ATTACHED_PROPERTIES)

#endif // APPPARAMETER_H
