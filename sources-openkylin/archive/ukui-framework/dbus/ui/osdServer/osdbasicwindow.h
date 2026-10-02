/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OSDBASICWINDOW_H
#define OSDBASICWINDOW_H

#include <QObject>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickItem> // 确保包含
#include <ukuiwindowhelper.h>
#include "datamanager.h"

#define HIDE_TIME_COMMON 3 * 1000
#define BLUR_SIZE 20

class OsdBasicWindow : public QObject
{
    Q_OBJECT
public:
    OsdBasicWindow(int width, int height, DataManager* manager, QObject *parent = nullptr);

    void setHideTime(uint time) { m_hideTime = time; }
    void setQuickViewSource(const QString& url);
    void setOsdSize(uint width, uint height);

    // [新增] 暴露 rootObject
    QQuickItem* rootObject() const {
        return m_osdView ? m_osdView->rootObject() : nullptr;
    }

    // [新增] 暴露 view 指针，以便外部访问 errors() 等
    QQuickView* view() const {
        return m_osdView.get();
    }

public slots:
    void show();
    void disableBlurEffect();

private:
    void initOsdView();
    void initWindowHelper();
    void setDataManager(DataManager* manager);
    void setGeometryByWindowSize(int width, int height);
    void blurWindow();

private:
    std::unique_ptr<QQuickView> m_osdView;
    UkuiWindowHelper* m_windowHelper;
    uint m_hideTime = HIDE_TIME_COMMON;
};

#endif // OSDBASICWINDOW_H

