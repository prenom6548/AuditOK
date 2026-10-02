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
 *
 */

#ifndef WIDGET_DELEGATE_H
#define WIDGET_DELEGATE_H

#include <QQuickItem>

namespace UkuiQuick {
class WidgetContainer;
class Widget;
class WidgetLoader;
}

namespace UkuiShortcut {
class WidgetDelegate : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString quickWidgetId READ quickWidgetId WRITE setQuickWidgetId NOTIFY quickWidgetIdChanged)
    Q_PROPERTY(qreal preferredWidth READ preferredWidth NOTIFY preferredWidthChanged)
    Q_PROPERTY(qreal preferredHeight READ preferredHeight NOTIFY preferredHeightChanged)
public:
    explicit WidgetDelegate(QQuickItem *parent = nullptr);
    ~WidgetDelegate();

    QString quickWidgetId() const;
    void setQuickWidgetId(const QString &id);

    qreal preferredWidth() const;
    qreal preferredHeight() const;

Q_SIGNALS:
    void quickWidgetIdChanged();
    void preferredWidthChanged();
    void preferredHeightChanged();

private:
    void loadContainer();
    void loadWidget();

    QString m_quickWidgetId;
    UkuiQuick::WidgetLoader *m_widgetLoader = nullptr;
    UkuiQuick::WidgetContainer *m_container = nullptr;
    UkuiQuick::Widget *m_widget = nullptr;
    qreal m_preferredWidth = 0;
    qreal m_preferredHeight = 0;

};
}

#endif //WIDGET_DELEGATE_H
