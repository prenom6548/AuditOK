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
 */

#ifndef WINDOWBLURHELPER_H
#define WINDOWBLURHELPER_H

#include <QObject>
#include <QWindow>
#include "window-helper.h"

namespace Sidebar {
class WindowBlurHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enable READ enable WRITE setEnable)
    //radius: 0-4000
    Q_PROPERTY(double radius READ radius WRITE setRadius NOTIFY radiusChanged)

public:
    explicit WindowBlurHelper(QObject *parent = nullptr);

    void setWindowProxy(UkuiQuick::WindowProxy* windowProxy);

    bool enable();
    void setEnable(bool enable);

    double radius();
    void setRadius(double radius);

    Q_INVOKABLE void setRegion(qreal x, qreal y, qreal w, qreal h, qreal radius);

private:
    inline bool updateBlurArea();
    void enableBlurBehind(WId window, bool enable, const QRegion &region, uint32_t radius);

private:
    UkuiQuick::WindowProxy* m_windowProxy = nullptr;
    QRegion                 m_region;
    bool                    m_enable = false;
    double                  m_radius = 0;

Q_SIGNALS:
    void radiusChanged();
};
}
#endif // WINDOWBLURHELPER_H
