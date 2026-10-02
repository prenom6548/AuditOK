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

#include "configmanager.h"
#include <QFileInfo>
#include <QProcess>

ConfigModel::ConfigModel(FcitxDbusProvider *provider, const QString &uri,
                         QObject *parent)
    : QObject(parent), m_provider(provider), m_uri(uri) {}

ConfigModel::~ConfigModel() {
  for (auto &group : m_configs) {
    group.reset();
  }
  m_configs.clear();

  if (m_configGroup) {
    m_configGroup.reset();
  }
}

QString ConfigModel::joinPath(const QString &path, const QString &option) {
  if (path.isEmpty()) {
    return option;
  }
  return QString("%1/%2").arg(path, option);
}

void ConfigModel::setupItems(std::unique_ptr<ConfigGroup> &group,
                             const QString &type, const QString &path) {
  if (!m_desc.contains(type)) {
    qDebug() << type << " type does not exists.";
  }
  const auto &options = m_desc[type];
  for (auto &option : options) {
    addConfigItem(group, option, joinPath(path, option.name()));
  }
  if (m_configGroup) {
    m_configs.push_back(std::move(m_configGroup));
  }
}

void ConfigModel::addConfigItem(std::unique_ptr<ConfigGroup> &group,
                                const FcitxQtConfigOption &option,
                                const QString &path) {
  if (auto item = ConfigItem::create(option, path, this)) {
    if (!group) {
      group = std::make_unique<ConfigGroup>(option.name(), "", this);
      group->addItem(std::move(item));
    } else {
      group->addItem(std::move(item));
    }
  } else if (m_desc.contains(option.type())) {
    auto ogPtr = std::make_unique<ConfigGroup>(option.name(),
                                               option.description(), this);
    setupItems(ogPtr, option.type(), path);
    m_configs.push_back(std::move(ogPtr));
  } else {
    qDebug() << "unkown type!" << option.type();
  }
}

const FcitxQtConfigOptionList *
ConfigModel::optionsForType(const QString &type) const {
  auto iter = m_desc.constFind(type);
  if (iter == m_desc.constEnd()) {
    return nullptr;
  }
  return &iter.value();
}

void ConfigModel::setValue(const QVariant &value) {
  if (!m_initialized) {
    return;
  }

  QVariantMap map;
  if (value.canConvert<QDBusArgument>()) {
    auto argument = qvariant_cast<QDBusArgument>(value);
    argument >> map;
  } else {
    map = value.toMap();
  }

  for (auto const &groups : m_configs) {
    for (auto const &item : groups->items()) {
      item->readValueFrom(map);
    }
  }
}

void ConfigModel::getConfig(const bool &sync) {
  if (!m_provider || !m_provider->dbusProxy() || m_uri.isEmpty()) {
    return;
  }
  qDebug() << "getConfig, sync:" << sync;
  auto call = m_provider->dbusProxy()->GetConfig(m_uri);
  auto watcher = new QDBusPendingCallWatcher(call, this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          &ConfigModel::requestConfigFinished);
  if (sync) {
    watcher->waitForFinished();
  }
}

void ConfigModel::restore() {
  for (auto const &groups : m_configs) {
    for (auto const &item : groups->items()) {
      item->restoreToDefault();
    }
  }
  emit configurationChanged();
}

void ConfigModel::save() const {
  if (!m_provider || !m_provider->dbusProxy() || m_uri.isEmpty()) {
    return;
  }

  QDBusVariant var(generateSaveValue());
  m_provider->dbusProxy()->SetConfig(m_uri, var);
}

void ConfigModel::requestConfigFinished(QDBusPendingCallWatcher *watcher) {
  watcher->deleteLater();
  QDBusPendingReply<QDBusVariant, FcitxQtConfigTypeList> reply = *watcher;
  if (reply.isError()) {
    qDebug() << "reply error!";
    return;
  }

  if (!m_initialized) {
    auto desc = reply.argumentAt<1>();
    if (!desc.size()) {
      return;
    }

    for (auto &type : desc) {
      m_desc[type.name()] = type.options();
    }
    m_rootType = desc[0].name();
    setupItems(m_configGroup, m_rootType, QString());
    m_initialized = true;
  }

  if (m_initialized) {
    setValue(reply.argumentAt<0>().variant());
  }
  emit configurationChanged();
}

QVariant ConfigModel::generateSaveValue() const {
  QVariantMap map;
  for (auto const &groups : m_configs) {
    for (auto const &item : groups->items()) {
      item->writeValueTo(map);
    }
  }
  return map;
}

void ConfigManager::saveImConfig() {
  if (m_imConfig) {
    m_imConfig->save();
  }
}

void ConfigManager::reloadImConfig() {
  if (m_imConfig) {
    m_imConfig->getConfig(true);
  }
}

void ConfigManager::restoreDefaultImConfig() {
  if (m_imConfig) {
    m_imConfig->restore();
  }
}

void ConfigManager::saveGlobalConfig() {
  if (m_gloalConfig) {
    m_gloalConfig->save();
  }
}

void ConfigManager::restoreDefaultExternalConfig() {
  if (m_externalConfig) {
    m_externalConfig->restore();
  }
}

void ConfigManager::saveExternalConfig() {
  if (m_externalConfig) {
    m_externalConfig->save();
  }
}

void ConfigManager::launchSogouConfigTool(const QString &uniqueName) {
  if (uniqueName.isEmpty()) {
    qWarning() << "uniqueName empty!";
    return;
  }

  QString program = "/opt/apps/com.cpis/bin/sogou-ime-setting.sh";
  QFileInfo scriptInfo(program);
  if (!scriptInfo.exists() || !scriptInfo.isFile() ||
      !scriptInfo.isExecutable()) {
    qWarning() << "Sogou config tool not found or not executable:" << program;
    return;
  }

  QString argIniPrefix = "/opt/apps/com.cpis/etc/modules/im/fcitx5/";
  QString argUidPrefix = "_CURRENT_USER_#im.module=fcitx5|im.name=";
  QStringList arguments = {"--ini", argIniPrefix + uniqueName + "/isp.ini",
                           "--uid", argUidPrefix + uniqueName};

  qDebug() << "program:" << program << "arguments:" << arguments;
  QProcess::startDetached(program, arguments);
}

ConfigManager::ConfigManager(FcitxDbusProvider *provider, QObject *parent)
    : QObject(parent), m_provider(provider) {
  getGlobalConfig(false);
}

void ConfigManager::getGlobalConfig(const bool &sync) {
  if (!m_gloalConfig) {
    m_gloalConfig =
        std::make_unique<ConfigModel>(m_provider, m_globalConfigUri, this);
    QObject::connect(m_gloalConfig.get(), &ConfigModel::configurationChanged,
                     this, &ConfigManager::globalConfigChanged);
    m_gloalConfig->getConfig(sync);
  }
}

void ConfigManager::getImConfig(const QString &uniqueName) {
  if (!m_imConfig) {
    m_imConfig = std::make_unique<ConfigModel>(
        m_provider, QString(m_imConfigUriPrefix + "%1").arg(uniqueName), this);
    QObject::connect(m_imConfig.get(), &ConfigModel::configurationChanged, this,
                     &ConfigManager::imConfigurationChanged);
    m_imConfig->getConfig(true);
  }
}

void ConfigManager::getExternalConfig(const QString &uri) {
  if (uri.isEmpty()) {
    return;
  }
  if (!m_externalConfig) {
    m_externalConfig = std::make_unique<ConfigModel>(m_provider, uri, this);
    m_externalConfig->getConfig(true);
  }
}

void ConfigManager::releaseImConfigResource() {
  if (m_imConfig) {
    QObject::disconnect(m_imConfig.get(), nullptr, this, nullptr);
    m_imConfig.reset();
  }
}

void ConfigManager::releaseExternalConfigResource() {
  if (m_externalConfig) {
    QObject::disconnect(m_externalConfig.get(), nullptr, this, nullptr);
    m_externalConfig.reset();
  }
}
