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
 */

#ifndef AUTOTESTS_WINDOWS_STUB_SHARED_ENGINE_VIEW_H
#define AUTOTESTS_WINDOWS_STUB_SHARED_ENGINE_VIEW_H

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>

namespace UkuiQuick {

class SharedEngineView : public QQuickWindow
{
    Q_OBJECT
public:
    enum ResizeMode { SizeViewToRootObject, SizeRootObjectToView };

    static QQmlEngine *sharedEngine();

    explicit SharedEngineView(QWindow *parent = nullptr);
    QQmlEngine *engine() const;
    QQmlContext *rootContext() const;
    QQuickItem *rootObject() const;
    void setResizeMode(SharedEngineView::ResizeMode resizeMode);
    SharedEngineView::ResizeMode resizeMode() const;
    void setTestRootObject(QQuickItem *item);

public Q_SLOTS:
    void setSource(const QUrl &source) const;

private:
    QQmlContext *m_context = nullptr;
    QQuickItem *m_rootObject = nullptr;
    ResizeMode m_resizeMode = SizeViewToRootObject;
    mutable QUrl m_source;
};

} // namespace UkuiQuick

#endif
