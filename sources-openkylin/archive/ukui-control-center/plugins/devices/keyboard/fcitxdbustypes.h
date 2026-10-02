/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _FCITXDBUSTYPES_H_
#define _FCITXDBUSTYPES_H_

#include <QDBusArgument>
#include <QList>
#include <QMetaType>
#include <type_traits>

void registerFcitxQtDBusTypes();

#define FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(TYPE) \
  class TYPE                                    \
  {                                             \
    Q_GADGET

#define FCITX5_QT_DECLARE_PROPERTY(TYPE, GETTER, SETTER) \
  Q_PROPERTY(TYPE GETTER READ GETTER WRITE SETTER)

#define FCITX5_QT_DECLARE_FIELD(TYPE, GETTER, SETTER)                         \
public:                                                                       \
  std::conditional_t<std::is_class<TYPE>::value, const TYPE &, TYPE> GETTER() \
      const                                                                   \
  {                                                                           \
    return GETTER##_;                                                         \
  }                                                                           \
  void SETTER(                                                                \
      std::conditional_t<std::is_class<TYPE>::value, const TYPE &, TYPE>      \
          value)                                                              \
  {                                                                           \
    GETTER##_ = value;                                                        \
  }                                                                           \
                                                                              \
private:                                                                      \
  TYPE GETTER##_ = TYPE();

#define FCITX5_QT_END_DECLARE_DBUS_TYPE(TYPE)                            \
  }                                                                      \
  ;                                                                      \
  typedef QList<TYPE> TYPE##List;                                        \
  QDBusArgument &operator<<(QDBusArgument &argument, const TYPE &value); \
  const QDBusArgument &operator>>(const QDBusArgument &argument, TYPE &value);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtStringKeyValue);
FCITX5_QT_DECLARE_PROPERTY(QString, key, setKey);
FCITX5_QT_DECLARE_PROPERTY(QString, value, setValue);
FCITX5_QT_DECLARE_FIELD(QString, key, setKey);
FCITX5_QT_DECLARE_FIELD(QString, value, setValue);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtStringKeyValue);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtInputMethodEntry);
FCITX5_QT_DECLARE_PROPERTY(QString, uniqueName, setUniqueName);
FCITX5_QT_DECLARE_PROPERTY(QString, name, setName);
FCITX5_QT_DECLARE_PROPERTY(QString, nativeName, setNativeName);
FCITX5_QT_DECLARE_PROPERTY(QString, icon, setIcon);
FCITX5_QT_DECLARE_PROPERTY(QString, label, setLabel);
FCITX5_QT_DECLARE_PROPERTY(QString, languageCode, setLanguageCode);
FCITX5_QT_DECLARE_PROPERTY(bool, configurable, setConfigurable);
FCITX5_QT_DECLARE_FIELD(QString, uniqueName, setUniqueName);
FCITX5_QT_DECLARE_FIELD(QString, name, setName);
FCITX5_QT_DECLARE_FIELD(QString, nativeName, setNativeName);
FCITX5_QT_DECLARE_FIELD(QString, icon, setIcon);
FCITX5_QT_DECLARE_FIELD(QString, label, setLabel);
FCITX5_QT_DECLARE_FIELD(QString, languageCode, setLanguageCode);
FCITX5_QT_DECLARE_FIELD(bool, configurable, setConfigurable);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtInputMethodEntry);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtFullInputMethodEntry);
FCITX5_QT_DECLARE_PROPERTY(QString, uniqueName, setUniqueName);
FCITX5_QT_DECLARE_PROPERTY(QString, name, setName);
FCITX5_QT_DECLARE_PROPERTY(QString, nativeName, setNativeName);
FCITX5_QT_DECLARE_PROPERTY(QString, icon, setIcon);
FCITX5_QT_DECLARE_PROPERTY(QString, label, setLabel);
FCITX5_QT_DECLARE_PROPERTY(QString, languageCode, setLanguageCode);
FCITX5_QT_DECLARE_PROPERTY(QString, addon, setAddon);
FCITX5_QT_DECLARE_PROPERTY(bool, configurable, setConfigurable);
FCITX5_QT_DECLARE_PROPERTY(QString, layout, setLayout);
FCITX5_QT_DECLARE_PROPERTY(QVariantMap, properties, setProperties);
FCITX5_QT_DECLARE_FIELD(QString, uniqueName, setUniqueName);
FCITX5_QT_DECLARE_FIELD(QString, name, setName);
FCITX5_QT_DECLARE_FIELD(QString, nativeName, setNativeName);
FCITX5_QT_DECLARE_FIELD(QString, icon, setIcon);
FCITX5_QT_DECLARE_FIELD(QString, label, setLabel);
FCITX5_QT_DECLARE_FIELD(QString, languageCode, setLanguageCode);
FCITX5_QT_DECLARE_FIELD(QString, addon, setAddon);
FCITX5_QT_DECLARE_FIELD(bool, configurable, setConfigurable);
FCITX5_QT_DECLARE_FIELD(QString, layout, setLayout);
FCITX5_QT_DECLARE_FIELD(QVariantMap, properties, setProperties);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtFullInputMethodEntry);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtVariantInfo);
FCITX5_QT_DECLARE_PROPERTY(QString, variant, setVariant);
FCITX5_QT_DECLARE_PROPERTY(QString, description, setDescription);
FCITX5_QT_DECLARE_PROPERTY(QStringList, languages, setLanguages);
FCITX5_QT_DECLARE_FIELD(QString, variant, setVariant);
FCITX5_QT_DECLARE_FIELD(QString, description, setDescription);
FCITX5_QT_DECLARE_FIELD(QStringList, languages, setLanguages);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtVariantInfo);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtLayoutInfo);
FCITX5_QT_DECLARE_PROPERTY(QString, layout, setLayout);
FCITX5_QT_DECLARE_PROPERTY(QString, description, setDescription);
FCITX5_QT_DECLARE_PROPERTY(QStringList, languages, setLanguages);
FCITX5_QT_DECLARE_PROPERTY(FcitxQtVariantInfoList, variants, setVariants);
FCITX5_QT_DECLARE_FIELD(QString, layout, setLayout);
FCITX5_QT_DECLARE_FIELD(QString, description, setDescription);
FCITX5_QT_DECLARE_FIELD(QStringList, languages, setLanguages);
FCITX5_QT_DECLARE_FIELD(FcitxQtVariantInfoList, variants, setVariants);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtLayoutInfo);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtConfigOption);
FCITX5_QT_DECLARE_PROPERTY(QString, name, setName);
FCITX5_QT_DECLARE_PROPERTY(QString, type, setType);
FCITX5_QT_DECLARE_PROPERTY(QString, description, setDescription);
FCITX5_QT_DECLARE_PROPERTY(QDBusVariant, defaultValue, setDefaultValue);
FCITX5_QT_DECLARE_PROPERTY(QVariantMap, properties, setProperties);
FCITX5_QT_DECLARE_FIELD(QString, name, setName);
FCITX5_QT_DECLARE_FIELD(QString, type, setType);
FCITX5_QT_DECLARE_FIELD(QString, description, setDescription);
FCITX5_QT_DECLARE_FIELD(QDBusVariant, defaultValue, setDefaultValue);
FCITX5_QT_DECLARE_FIELD(QVariantMap, properties, setProperties);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtConfigOption);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtConfigType);
FCITX5_QT_DECLARE_PROPERTY(QString, name, setName);
FCITX5_QT_DECLARE_PROPERTY(FcitxQtConfigOptionList, options, setOptions);
FCITX5_QT_DECLARE_FIELD(QString, name, setName);
FCITX5_QT_DECLARE_FIELD(FcitxQtConfigOptionList, options, setOptions);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtConfigType);

FCITX5_QT_BEGIN_DECLARE_DBUS_TYPE(FcitxQtHotkeyInfo);
FCITX5_QT_DECLARE_PROPERTY(QString, category, setCategory);
FCITX5_QT_DECLARE_PROPERTY(QString, name, setName);
FCITX5_QT_DECLARE_PROPERTY(QString, uniqueName, setUniqueName);
FCITX5_QT_DECLARE_PROPERTY(QString, description, setDescription);
FCITX5_QT_DECLARE_PROPERTY(QStringList, keys, setKeys);
FCITX5_QT_DECLARE_FIELD(QString, category, setCategory);
FCITX5_QT_DECLARE_FIELD(QString, name, setName);
FCITX5_QT_DECLARE_FIELD(QString, uniqueName, setUniqueName);
FCITX5_QT_DECLARE_FIELD(QString, description, setDescription);
FCITX5_QT_DECLARE_FIELD(QStringList, keys, setKeys);
FCITX5_QT_END_DECLARE_DBUS_TYPE(FcitxQtHotkeyInfo);

Q_DECLARE_METATYPE(FcitxQtStringKeyValue)
Q_DECLARE_METATYPE(FcitxQtStringKeyValueList)

Q_DECLARE_METATYPE(FcitxQtInputMethodEntry)
Q_DECLARE_METATYPE(FcitxQtInputMethodEntryList)

Q_DECLARE_METATYPE(FcitxQtFullInputMethodEntry)
Q_DECLARE_METATYPE(FcitxQtFullInputMethodEntryList)

Q_DECLARE_METATYPE(FcitxQtVariantInfo)
Q_DECLARE_METATYPE(FcitxQtVariantInfoList)

Q_DECLARE_METATYPE(FcitxQtLayoutInfo)
Q_DECLARE_METATYPE(FcitxQtLayoutInfoList)

Q_DECLARE_METATYPE(FcitxQtConfigOption)
Q_DECLARE_METATYPE(FcitxQtConfigOptionList)

Q_DECLARE_METATYPE(FcitxQtConfigType)
Q_DECLARE_METATYPE(FcitxQtConfigTypeList)

Q_DECLARE_METATYPE(FcitxQtHotkeyInfo)
Q_DECLARE_METATYPE(FcitxQtHotkeyInfoList)

#endif //_FCITXQTDBUSTYPES_H_
