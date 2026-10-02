/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef REGION_H
#define REGION_H

#include <QObject>
#include <QQmlListProperty>
#include <QRegion>

namespace UkuiQuick {

class RectRegionPrivate;
class RectRegion : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(int y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)
public:
    explicit RectRegion(QObject *parent = nullptr);
    ~RectRegion();
    void setX(int x);
    int x() const;

    void setY(int y);
    int y() const;

    void setWidth(int width);
    int width() const;

    void setHeight(int height);
    int height() const;

    void setRadius(int radius);
    int radius() const;

Q_SIGNALS:
    void xChanged();
    void yChanged();
    void widthChanged();
    void heightChanged();
    void radiusChanged();

private:
    RectRegionPrivate *d = nullptr;
};

class RegionPrivate;
class Region : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRegion region READ region NOTIFY regionChanged)

public:
    explicit Region(QObject *parent = nullptr);
    Q_INVOKABLE void appendRect(UkuiQuick::RectRegion *rect);
    Q_INVOKABLE void appendRectList(QList<UkuiQuick::RectRegion *> rects);
    Q_INVOKABLE void removeRect(UkuiQuick::RectRegion *rect);
    Q_INVOKABLE void clear();
    Q_INVOKABLE  QRegion region() const;

Q_SIGNALS:
    void regionChanged();

protected:
    bool event(QEvent *event) override;

private:
    void addRegionFromRect(UkuiQuick::RectRegion *rect);
    RegionPrivate *d = nullptr;
};

}

#endif //REGION_H
