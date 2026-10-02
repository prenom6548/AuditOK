/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 */

#include "blur-region-helper.h"
#include <QPainterPath>

namespace UkuiPanel {
BlurRegionHelper::BlurRegionHelper(QObject *parent) : QObject(parent)
{
}

BlurRegionHelper::~BlurRegionHelper()
{
}
//TODO: find a better way to set blur behind effect
void BlurRegionHelper::addItem(QQuickItem *item, int radius )
{
    if(!item || (m_items.contains(item) && m_items.value(item) == radius)) {
        return;
    }
    m_items.insert(item, radius);
    connect(item, &QQuickItem::destroyed, this, [&, item]() {
        removeItem(item);
    });
    requestUpdate(item);
}

void BlurRegionHelper::removeItem(QQuickItem *item)
{
    if(!item) {
        return;
    }
    item->disconnect(this);
    m_items.remove(item);
    m_region.remove(item);
    m_pendingUpdateItems.removeAll(item);
    notifyRegionUpdated();
}

bool BlurRegionHelper::event(QEvent* event)
{
    if (event->type() == QEvent::UpdateRequest) {
        if(!m_pendingUpdateItems.isEmpty()) {
            for(auto item : m_pendingUpdateItems) {
                updateRegion(item);
            }
            m_pendingUpdateItems.clear();
            notifyRegionUpdated();
        }
    }

    return QObject::event(event);
}

void BlurRegionHelper::requestUpdate(QQuickItem* item)
{
    if(item) {
        if(!m_pendingUpdateItems.contains(item)) {
            m_pendingUpdateItems.append(item);
            QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
        }
    } else {
        for(auto item : m_items.keys()) {
            requestUpdate(item);
        }
    }
}

void BlurRegionHelper::updateRegion(QQuickItem* item)
{
    if(item) {
        QPainterPath path;
        path.addRoundedRect(item->mapToScene(QPointF(0, 0)).x(),  item->mapToScene(QPointF(0, 0)).y(), item->width(), item->height(), m_items.value(item), m_items.value(item));
        m_region.insert(item, path.toFillPolygon().toPolygon());
    }
}

void BlurRegionHelper::notifyRegionUpdated()
{
    Q_EMIT regionUpdated(region());
}

QRegion BlurRegionHelper::region()
{
    QRegion region;
    for(const QRegion &r : m_region) {
        region = region.united(r);
    }
    return region;
}
} // UkuiPanel