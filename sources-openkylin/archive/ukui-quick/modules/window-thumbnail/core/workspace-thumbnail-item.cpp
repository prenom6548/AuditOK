/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: sundagao <sundagao@kylinos.cn>
 *
 */

#include "workspace-thumbnail-item.h"
#include "wlcom-thumbnail.h"
using namespace UkuiThumbnail;

WorkspaceThumbnailItem::WorkspaceThumbnailItem(QQuickItem* parent)
    : ThumbnailItem(parent)
{
}

UkuiThumbnail::Thumbnail* WorkspaceThumbnailItem::createThumbnail()
{
    auto* thumbnail = new WlcomThumbnail(this);
    thumbnail->setType(WlcomThumbnail::Type::Workspace);
    return thumbnail;
}

QString WorkspaceThumbnailItem::outputId() const
{
    return m_outputId;
}

void WorkspaceThumbnailItem::setOutputId(const QString& outputId)
{
    if(m_outputId == outputId) {
        return;
    }
    m_outputId = outputId;
    thumbnail()->setOutputId(outputId);
    active(shouldBeActive());
    Q_EMIT outputIdChanged();
}
