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

#ifndef UKUI_QUICK_WORKSPACETHUMBNAILITEM_H
#define UKUI_QUICK_WORKSPACETHUMBNAILITEM_H

#include "thumbnail-item.h"
namespace UkuiThumbnail
{

    class WorkspaceThumbnailItem : public ThumbnailItem
    {
        Q_OBJECT
        Q_PROPERTY(QString outputId READ outputId WRITE setOutputId NOTIFY outputIdChanged)
    public:
        WorkspaceThumbnailItem(QQuickItem *parent = nullptr);
        ~WorkspaceThumbnailItem() override = default;

        UkuiThumbnail::Thumbnail* createThumbnail() override;

        QString outputId() const;
        void setOutputId(const QString &outputId);
    Q_SIGNALS:
        void outputIdChanged();
    private:
        QString m_outputId;
    };
}

#endif //UKUI_QUICK_WORKSPACETHUMBNAILITEM_H