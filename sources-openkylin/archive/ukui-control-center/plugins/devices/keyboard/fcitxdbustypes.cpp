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

#include "fcitxdbustypes.h"

#include <QDBusMetaType>

#define FCITX5_QT_DEFINE_DBUS_TYPE(TYPE)       \
  qRegisterMetaType<TYPE>(#TYPE);              \
  qDBusRegisterMetaType<TYPE>();               \
  qRegisterMetaType<TYPE##List>(#TYPE "List"); \
  qDBusRegisterMetaType<TYPE##List>();

void registerFcitxQtDBusTypes()
{
  FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtStringKeyValue);
  FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtInputMethodEntry);
  FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtFullInputMethodEntry);
  FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtLayoutInfo);
  FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtVariantInfo);
  FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtConfigOption);
  FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtConfigType);
  FCITX5_QT_DEFINE_DBUS_TYPE(FcitxQtHotkeyInfo);
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtStringKeyValue &arg)
{
  argument.beginStructure();
  argument << arg.key();
  argument << arg.value();
  argument.endStructure();
  return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtStringKeyValue &arg)
{
  QString key, value;
  argument.beginStructure();
  argument >> key >> value;
  argument.endStructure();
  arg.setKey(key);
  arg.setValue(value);
  return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtInputMethodEntry &arg)
{
  argument.beginStructure();
  argument << arg.uniqueName();
  argument << arg.name();
  argument << arg.nativeName();
  argument << arg.icon();
  argument << arg.label();
  argument << arg.languageCode();
  argument << arg.configurable();
  argument.endStructure();
  return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtInputMethodEntry &arg)
{
  QString uniqueName, name, nativeName, icon, label, languageCode;
  bool configurable;
  argument.beginStructure();
  argument >> uniqueName >> name >> nativeName >> icon >> label >>
      languageCode >> configurable;
  argument.endStructure();
  arg.setUniqueName(uniqueName);
  arg.setName(name);
  arg.setNativeName(nativeName);
  arg.setIcon(icon);
  arg.setLabel(label);
  arg.setLanguageCode(languageCode);
  arg.setConfigurable(configurable);
  return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtFullInputMethodEntry &arg)
{
  argument.beginStructure();
  argument << arg.uniqueName();
  argument << arg.name();
  argument << arg.nativeName();
  argument << arg.icon();
  argument << arg.label();
  argument << arg.languageCode();
  argument << arg.addon();
  argument << arg.configurable();
  argument << arg.layout();
  argument << arg.properties();
  argument.endStructure();
  return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtFullInputMethodEntry &arg)
{
  QString uniqueName, name, nativeName, icon, label, languageCode, addon,
      layout;
  bool configurable;
  QVariantMap properties;
  argument.beginStructure();
  argument >> uniqueName >> name >> nativeName >> icon >> label >>
      languageCode >> addon >> configurable >> layout >> properties;
  argument.endStructure();
  arg.setUniqueName(uniqueName);
  arg.setName(name);
  arg.setNativeName(nativeName);
  arg.setIcon(icon);
  arg.setLabel(label);
  arg.setLanguageCode(languageCode);
  arg.setAddon(addon);
  arg.setConfigurable(configurable);
  arg.setLayout(layout);
  arg.setProperties(properties);
  return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtVariantInfo &arg)
{
  argument.beginStructure();
  argument << arg.variant();
  argument << arg.description();
  argument << arg.languages();
  argument.endStructure();
  return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtVariantInfo &arg)
{
  QString variant, description;
  QStringList languages;
  argument.beginStructure();
  argument >> variant >> description >> languages;
  argument.endStructure();
  arg.setVariant(variant);
  arg.setDescription(description);
  arg.setLanguages(languages);
  return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtLayoutInfo &arg)
{
  argument.beginStructure();
  argument << arg.layout();
  argument << arg.description();
  argument << arg.languages();
  argument << arg.variants();
  argument.endStructure();
  return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtLayoutInfo &arg)
{
  QString layout, description;
  QStringList languages;
  FcitxQtVariantInfoList variants;
  argument.beginStructure();
  argument >> layout >> description >> languages >> variants;
  argument.endStructure();
  arg.setLayout(layout);
  arg.setDescription(description);
  arg.setLanguages(languages);
  arg.setVariants(variants);
  return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtConfigOption &arg)
{
  argument.beginStructure();
  argument << arg.name();
  argument << arg.type();
  argument << arg.description();
  argument << arg.defaultValue();
  argument << arg.properties();
  argument.endStructure();
  return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtConfigOption &arg)
{
  QString name, description, type;
  QDBusVariant defaultValue;
  QVariantMap properties;
  argument.beginStructure();
  argument >> name >> type >> description >> defaultValue >> properties;
  argument.endStructure();
  arg.setName(name);
  arg.setType(type);
  arg.setDescription(description);
  arg.setDefaultValue(defaultValue);
  arg.setProperties(properties);
  return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtConfigType &arg)
{
  argument.beginStructure();
  argument << arg.name();
  argument << arg.options();
  argument.endStructure();
  return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtConfigType &arg)
{
  QString name;
  FcitxQtConfigOptionList options;
  argument.beginStructure();
  argument >> name >> options;
  argument.endStructure();
  arg.setName(name);
  arg.setOptions(options);
  return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const FcitxQtHotkeyInfo &arg)
{
  argument.beginStructure();
  argument << arg.category();
  argument << arg.name();
  argument << arg.uniqueName();
  argument << arg.description();
  argument << arg.keys();
  argument.endStructure();
  return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                FcitxQtHotkeyInfo &arg)
{
  QString category, name, uniqueName, description;
  QStringList keys;
  argument.beginStructure();
  argument >> category >> name >> uniqueName >> description >> keys;
  argument.endStructure();
  arg.setCategory(category);
  arg.setName(name);
  arg.setUniqueName(uniqueName);
  arg.setDescription(description);
  arg.setKeys(keys);
  return argument;
}
