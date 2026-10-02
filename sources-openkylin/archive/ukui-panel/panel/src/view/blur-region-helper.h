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

#ifndef UKUI_PANEL_BLUR_REGION_HELPER_H
#define UKUI_PANEL_BLUR_REGION_HELPER_H

#include <QObject>
#include <QQuickItem>
#include <QRegion>
namespace UkuiPanel {

class BlurRegionHelper : public QObject
{
    Q_OBJECT
public:
    BlurRegionHelper(QObject *parent = nullptr);
    ~BlurRegionHelper();
    Q_INVOKABLE void addItem(QQuickItem *item, int radius = 0);
    Q_INVOKABLE void removeItem(QQuickItem *item);
    Q_INVOKABLE void requestUpdate(QQuickItem *item = nullptr);
    QRegion region();

Q_SIGNALS:
    void regionUpdated(QRegion region);

protected:
    bool event(QEvent* event) override;

private Q_SLOTS:
    void updateRegion(QQuickItem *item);
    void notifyRegionUpdated();

private:
    QMap<QQuickItem*, int> m_items;
    QMap<QQuickItem*, QRegion> m_region;
    QList<QQuickItem*> m_pendingUpdateItems;
};

} // UkuiPanel

#endif //UKUI_PANEL_BLUR_REGION_HELPER_H
