/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef ACCESSIBLEFORMAT_H
#define ACCESSIBLEFORMAT_H

#include <QApplication>
#include <QFileInfo>
#include <QString>
#include <QWidget>
#include <type_traits>

/**
 * @brief 设置控件属性
 * @since 2.5.2.0
 */

namespace kdk
{

template<typename T>
QString combineAccessibleName(T *obj, QString widgetName, const QString &pluginName, const QString &extend = QString())
{
    if (!obj)
        return QString();
    if (!std::is_base_of<QObject, T>::value)
        return QString();
    QFileInfo cmdInfo(QApplication::arguments().at(0));
    QString accessibleName = cmdInfo.fileName();
    accessibleName += "_";
    if (!pluginName.isEmpty()) {
        accessibleName += pluginName;
        accessibleName += "_";
    }
    accessibleName += obj->metaObject()->className();
    accessibleName += "_";
    accessibleName += widgetName.remove(QRegExp("[&*]"));
    if (!extend.isEmpty()) {
        accessibleName += "_";
        accessibleName += extend;
    }
    return accessibleName;
}

template<typename T>
QString combineAccessibleDescription(T *obj, const QString &widgetName)
{
    if (!obj)
        return QString();
    if (!std::is_base_of<QObject, T>::value)
        return QString();
    QFileInfo cmdInfo(QApplication::arguments().at(0));
    return QString("[%1] is [%2] type in process:[%3]")
        .arg(widgetName)
        .arg(obj->metaObject()->className())
        .arg(cmdInfo.fileName());
}

template<typename T>
class AccessInfoHelper
{
public:
    AccessInfoHelper(T *widget)
        : m_widget(widget)
    {
    }

    void setObjectInfo(const QString &widgetName, const QString &pluginName, const QString &extend = QString())
    {
        if (m_widget->objectName().isEmpty())
            m_widget->setObjectName(combineAccessibleName<T>(m_widget, widgetName, pluginName, extend));
    }

    void setAllAttribute(const QString &widgetName, const QString &pluginName = QString(), const QString &extend = QString(), const QString &description = QString())
    {
        if (!m_widget)
            return;
        setObjectInfo(widgetName, pluginName, extend);
        if (std::is_base_of<QWidget, T>::value) {
            m_widget->setAccessibleName(combineAccessibleName<T>(m_widget, widgetName, pluginName, extend));
            if (!description.isEmpty()) {
                m_widget->setAccessibleDescription(description);
            } else {
                m_widget->setAccessibleDescription(combineAccessibleDescription<T>(m_widget, widgetName));
            }
        }
    }

private:
    T *m_widget;
};

template<typename T>
AccessInfoHelper<T> getHandle(T *widget)
{
    return AccessInfoHelper<T>(widget);
}

#define KDK_ALL_INFO_FORMAT(widget, description) getHandle(widget).setAllAttribute(#widget, "", "", description)
#define KDK_EXTEND_ALL_INFO_FORMAT(widget, pluginName, extend, description) getHandle(widget).setAllAttribute(#widget, pluginName, extend, description)

#define KDK_OBJ_INFO_FORMAT(obj) getHandle(obj).setObjectInfo(#obj, "")
#define KDK_EXTEND_OBJ_INFO_FORMAT(obj, pluginName, extend) getHandle(obj).setObjectInfo(#obj, pluginName, extend)

}
#endif // ACCESSIBLEFORMAT_H
