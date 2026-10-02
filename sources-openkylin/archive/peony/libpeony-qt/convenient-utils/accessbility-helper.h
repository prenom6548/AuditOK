/*
 * Peony-Qt
 *
 * Copyright (C) 2024, Tianjin KYLIN Information Technology Co., Ltd.
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
 * @author: Renyg <renyangguang@kylinos.cn>
 * @date:   2024-09-05 15:25
 */

#ifndef ACCESSBILITYHELPER_H
#define ACCESSBILITYHELPER_H

#include <functional>
#include <type_traits>
#include <QObject>

/**
 * @brief Provides methods to set accessibility properties for QObject and its subclasses.
 *
 * This class is a helper that simplifies the process of setting accessibility properties
 * for QObject and its subclasses, such as QWidget. It provides a convenient way to set
 * the object name, accessible name, and accessible description for a given widget.
 *
 * @tparam WidgetType The type of widget, must be QObject or a subclass of QObject.
 */
template<typename WidgetType, typename = typename std::enable_if<std::is_base_of<QObject, WidgetType>::value>::type >
class AccessibilityHelper {
public:
    /**
     * @brief Constructs an AccessibilityHelper instance for the given widget.
     *
     * @param widget Pointer to the widget to be made accessible.
     */
    AccessibilityHelper(WidgetType* widget): m_widget(widget) {}

    /**
     * @brief Sets the accessibility properties for the widget.
     *
     * This method sets the object name, accessible name, and accessible description
     * for the widget. If the instance count is greater than 1, the object name will
     * be appended with the instance count.
     *
     * @param name The base name for the widget.
     * @param instanceCount The instance count of the widget, if applicable.
     */
    void setAll(const QString& name, uint8_t instanceCount) {
        QString fullName = name;
        if (instanceCount > 1) {
            fullName = QString("%1_%2").arg(name).arg(instanceCount);
        }

        if (m_widget->objectName().isEmpty()) {
            m_widget->setObjectName(fullName);
        }

        setAccessibilityImpl(name, instanceCount, fullName, std::is_base_of<QWidget, WidgetType>());
    }

private:
    /**
     * @brief Implements the logic to set the accessibility properties for QWidget-based widgets.
     *
     * This method sets the accessible name and accessible description for the widget.
     * The accessible description is constructed based on the widget's name and type.
     *
     * The object name is set as "ProductName_WidgetType_WidgetName_N", where N is the instance count.
     * The accessible name and description are set based on the object name.
     *
     * @tparam T The type of the widget, must be a subclass of QWidget.
     * @param name The base name for the widget.
     * @param instanceCount The instance count of the widget, if applicable.
     * @param fullName The full name of the widget, including the instance count if applicable.
     * @param true_type A dummy parameter to enable the overload resolution.
     */
    template<typename T = WidgetType, typename = typename std::enable_if<std::is_base_of<QWidget, T>::value>::type>
    void setAccessibilityImpl(const QString& name, uint8_t instanceCount, const QString& fullName, std::true_type) {
        m_widget->setAccessibleName(fullName);

        QStringList parts = name.split("_");
        QString accessibleDesc;
        if (parts.size() >= 3) {
            QString suffixPart = parts.mid(2).join("_");
            if (instanceCount > 1) {
                accessibleDesc = QString("%1 %4 is %2 type in %3").arg(suffixPart).arg(parts[1]).arg(parts[0]).arg(instanceCount);
            } else {
                accessibleDesc = QString("%1 is %2 type in %3").arg(suffixPart).arg(parts[1]).arg(parts[0]);
            }
        }
        m_widget->setAccessibleDescription(accessibleDesc);
    }

    /**
     * @brief Implements the logic to set the accessibility properties for non-QWidget-based widgets.
     *
     * This method is a no-op, as setting accessibility properties is only applicable for QWidget-based widgets.
     *
     * @tparam T The type of the widget, must not be a subclass of QWidget.
     * @param name The base name for the widget (unused).
     * @param instanceCount The instance count of the widget (unused).
     * @param fullName The full name of the widget (unused).
     * @param false_type A dummy parameter to enable the overload resolution.
     */
    template<typename T = WidgetType, typename = typename std::enable_if<!std::is_base_of<QWidget, T>::value>::type>
    void setAccessibilityImpl(const QString&, uint8_t, const QString&, std::false_type) {

    }

private:
    WidgetType* m_widget; ///< Pointer to the widget being made accessible
};


/**
 * @brief Helper function to create an AccessibilityHelper instance.
 *
 * This function simplifies the creation of an AccessibilityHelper instance
 * for the given widget. It ensures that the widget type is QObject or a
 * subclass of QObject.
 *
 * @tparam WidgetType The type of widget, must be QObject or a subclass of QObject.
 * @param widget Pointer to the widget to be made accessible.
 * @return An AccessibilityHelper instance for the given widget.
 */
template<typename WidgetType, typename = typename std::enable_if<std::is_base_of<QObject, WidgetType>::value>::type >
AccessibilityHelper<WidgetType> makeAccessible(WidgetType* widget) {
    return AccessibilityHelper<WidgetType>(widget);
}

#endif // ACCESSBILITYHELPER_H
