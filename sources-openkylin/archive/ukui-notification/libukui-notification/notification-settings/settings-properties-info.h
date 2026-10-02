/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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

#ifndef SETTINGS_PROPERTIES_INFO_H
#define SETTINGS_PROPERTIES_INFO_H

#include <QString>
#include <QVariant>
#include <QObject>
#include "settings-properties.h"

#include <memory>

namespace UkuiNotification {

class SettingsPropertiesInfoPrivate;
/**
 * \class SettingsPropertiesInfo SettingsPropertiesInfo.h
 */
class Q_DECL_IMPORT SettingsPropertiesInfo : public QObject
{
    Q_OBJECT
public:
    SettingsPropertiesInfo();
    SettingsPropertiesInfo(SettingsProperty::Property property);
    SettingsPropertiesInfo(const SettingsPropertiesInfo& pi);
    ~SettingsPropertiesInfo();

    SettingsPropertiesInfo& operator=(const SettingsPropertiesInfo& rhs);
    bool operator==(const SettingsPropertiesInfo& rhs) const;

    /**
     * The enumeration which represents this property
     */
    SettingsProperty::Property property() const;

    /**
     * The internal unique name used to refer to the property
     */
    QString name() const;

    /**
     * A user visible name of the property
     */
    QString displayName() const;

    /**
     * The type the value of this property should be.
     * Eg - Property::Height should be an integer
     */
    QMetaType::Type valueType() const;
    QString defaultValue() const;
    QString valueToString(const QVariant &value) const;

    /**
     * Construct a SettingsPropertiesInfo from the internal property name.
     * The internal property name is case insensitive
     */
    static SettingsPropertiesInfo fromName(const QString& name);

private:
    const std::unique_ptr<SettingsPropertiesInfoPrivate> d;
};

} // namespace
Q_DECLARE_METATYPE(UkuiNotification::SettingsPropertiesInfo)


#endif // SETTINGS_PROPERTIES_INFO_H
