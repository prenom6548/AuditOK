/*
*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef WIDGET_SHELL_H
#define WIDGET_SHELL_H

#include <QQuickItem>
#include <QEvent>
#include <qmutex.h>

namespace UkuiQuick {
class WidgetContainer;
class Widget;
class WidgetLoader;
}

namespace TaskManager {
class WidgetUpdateEvent : public QEvent
{
public:
    WidgetUpdateEvent(const QString &appId, const QString &widgetId) : QEvent(static_cast<QEvent::Type>(eventType())), m_appId(appId), m_widgetId(widgetId) {}
    QString appId() const {return m_appId;};
    QString widgetId() const {return m_widgetId;};
    static int eventType();
private:
    QString m_appId;
    QString m_widgetId;
    static int m_eventType;
};

class WidgetInfo : public QObject
{
    Q_OBJECT
public:
    static WidgetInfo *self();

    bool hasWidgetFor(const QString &appId);
    QString widgetFor(const QString &appId);
    UkuiQuick::Widget *loadWidget(const QString &id);

Q_SIGNALS:
    void widgetInfoUpdate(const QString &appid, const QString &widgetId);

protected:
    void customEvent(QEvent* event) override;

private:
    explicit WidgetInfo(QObject *parent = nullptr);

    UkuiQuick::WidgetLoader *m_widgetLoader = nullptr;
    QMap<QString, QString> m_quickWidgets;
    QMutex m_mutex;
};

class WidgetDelegate : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString quickWidgetId READ quickWidgetId WRITE setQuickWidgetId NOTIFY quickWidgetIdChanged)
    Q_PROPERTY(int preferredWidth READ preferredWidth NOTIFY preferredWidthChanged)

public:
    explicit WidgetDelegate(QQuickItem *parent = nullptr);
    ~WidgetDelegate();

    QString quickWidgetId() const;
    void setQuickWidgetId(const QString &id);

    int preferredWidth();

private:
    void loadContainer();
    void loadWidget();

Q_SIGNALS:
    void quickWidgetIdChanged();
    void preferredWidthChanged();

private:
    QString m_quickWidgetId;
    UkuiQuick::Widget *m_widget = nullptr;
    UkuiQuick::WidgetContainer *m_container = nullptr;
    int m_preferredWidth = 0;
};
}


#endif //WIDGET_SHELL_H
