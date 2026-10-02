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
#include "region.h"

#include <QCoreApplication>
#include <QEvent>
#include <QPainterPath>

namespace UkuiQuick {
class RectRegionPrivate
{
public:
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
    int m_radius = 0;
};

RectRegion::RectRegion(QObject *parent) : QObject(parent), d(new RectRegionPrivate)
{
}

RectRegion::~RectRegion()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

void RectRegion::setX(int x)
{
    if(d->m_x == x) {
        return;
    }
    d->m_x = x;
    Q_EMIT xChanged();
}

int RectRegion::x() const
{
    return d->m_x;
}

void RectRegion::setY(int y)
{
    if(d->m_y == y) {
        return;
    }
    d->m_y = y;
    Q_EMIT yChanged();
}

int RectRegion::y() const
{
    return d->m_y;
}

void RectRegion::setWidth(int width)
{
    if(d->m_width == width) {
        return;
    }
    d->m_width = width;
    Q_EMIT widthChanged();
}

int RectRegion::width() const
{
    return d->m_width;
}

void RectRegion::setHeight(int height)
{
    if(d->m_height == height) {
        return;
    }
    d->m_height = height;
    Q_EMIT heightChanged();
}

int RectRegion::height() const
{
    return d->m_height;
}

void RectRegion::setRadius(int radius)
{
    if(d->m_radius == radius) {
        return;
    }
    d->m_radius = radius;
    Q_EMIT radiusChanged();
}

int RectRegion::radius() const
{
    return d->m_radius;
}

class RegionPrivate : public QObject
{
    Q_OBJECT
public:
    RegionPrivate(Region *parent = nullptr);
    void updateSingleRect(RectRegion* rect);
    void removeSingleRect(RectRegion* rect);

    QRegion m_region;
    Region *q = nullptr;
    QHash<RectRegion*, QRegion> m_rectRegionCache;
};

RegionPrivate::RegionPrivate(Region* parent) : QObject(parent), q(parent)
{
}

void RegionPrivate::updateSingleRect(RectRegion* rect)
{
    if (!rect) {
        return;
    }

    QPainterPath path;
    path.addRoundedRect(rect->x(), rect->y(), rect->width(), rect->height(), rect->radius(), rect->radius());
    QRegion newRectRegion(path.toFillPolygon().toPolygon());

    m_rectRegionCache[rect] = newRectRegion;
    m_region = QRegion();
    for (auto it = m_rectRegionCache.begin(); it != m_rectRegionCache.end(); ++it) {
        m_region += it.value();
    }

    Q_EMIT q->regionChanged();
}

void RegionPrivate::removeSingleRect(RectRegion* rect)
{
    if (!rect) {
        return;
    }

    m_rectRegionCache.remove(rect);
    m_region = QRegion();
    for (auto it = m_rectRegionCache.begin(); it != m_rectRegionCache.end(); ++it) {
        m_region += it.value();
    }

    Q_EMIT q->regionChanged();
}

Region::Region(QObject* parent) : QObject(parent), d(new RegionPrivate(this))
{
}

bool Region::event(QEvent* event)
{
    return QObject::event(event);
}

void Region::addRegionFromRect(UkuiQuick::RectRegion* rect)
{
    if (!rect || d->m_rectRegionCache.contains(rect)) {
        return;
    }

    // 增加一个圆角矩形
    QPainterPath path;
    path.addRoundedRect(rect->x(), rect->y(), rect->width(), rect->height(), rect->radius(), rect->radius());
    QRegion rectRegion(path.toFillPolygon().toPolygon());
    d->m_region += rectRegion;
    d->m_rectRegionCache[rect] = rectRegion;

    connect(rect, &RectRegion::xChanged, d, [this, rect]() { d->updateSingleRect(rect); });
    connect(rect, &RectRegion::yChanged, d, [this, rect]() { d->updateSingleRect(rect); });
    connect(rect, &RectRegion::widthChanged, d, [this, rect]() { d->updateSingleRect(rect); });
    connect(rect, &RectRegion::heightChanged, d, [this, rect]() { d->updateSingleRect(rect); });
    connect(rect, &RectRegion::radiusChanged, d, [this, rect]() { d->updateSingleRect(rect); });
}

void Region::appendRect(UkuiQuick::RectRegion* rect)
{
    addRegionFromRect(rect);
    Q_EMIT regionChanged();
}

void Region::appendRectList(QList<UkuiQuick::RectRegion*> rects)
{
    for (auto rect : rects) {
        addRegionFromRect(rect);
    }
    Q_EMIT regionChanged();
}

void Region::removeRect(UkuiQuick::RectRegion* rect)
{
    if (!rect) {
        return;
    }
    rect->disconnect(d);
    d->removeSingleRect(rect);
}

void Region::clear()
{
    for (const UkuiQuick::RectRegion* rect : d->m_rectRegionCache.keys())
    {
        rect->disconnect(d);
    }
    d->m_rectRegionCache.clear();
    d->m_region = QRegion();
    Q_EMIT regionChanged();
}

QRegion Region::region() const
{
    return d->m_region;
}

}
#include "region.moc"
