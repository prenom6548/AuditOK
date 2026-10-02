/*
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
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

//
// Created by hxf on 22-8-8.
//

#ifndef UKUI_SEARCH_ICON_ITEM_H
#define UKUI_SEARCH_ICON_ITEM_H

#include <QQuickPaintedItem>
#include <QIcon>

class ThemeIcon : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ getIcon)
    Q_PROPERTY(QString source READ getSource WRITE setSource)
    Q_PROPERTY(QString fallback READ getFallBack WRITE setFallBack)
    Q_PROPERTY(bool disable READ disable WRITE setDisable)
    Q_PROPERTY(int radius READ radius WRITE setRadius)
    Q_PROPERTY(bool highLight READ isHighLight WRITE setHighLight)
    Q_PROPERTY(bool forceHighlight READ isForceHighlight WRITE setForceHighLight)

public:
    explicit ThemeIcon(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    QIcon getIcon();
    void setIcon(const QIcon& icon);
    void setIcon(const QPixmap& pixmap);

    QString getSource();
    void setSource(const QString& source);

    QString getFallBack();
    void setFallBack(const QString &fallback);

    bool isHighLight() const;
    void setHighLight(bool highLight);

    bool isForceHighlight() const;
    void setForceHighLight(bool force);

    bool disable() const;
    void setDisable(bool disable);

    int radius();
    void setRadius(int radius);

private:
    void readImage(const QString &path);
    bool isPixmapPureColor(const QPixmap &pixmap);

private:
    bool m_disabled = false;
    int  m_radius = 0;
    bool m_highLight = false;
    bool m_forceHighlight = false;
    QIcon m_rawIcon;
    QString m_source;
    QString m_fallback;

    static QColor symbolicColor;
};

#endif //UKUI_SEARCH_ICON_ITEM_H
