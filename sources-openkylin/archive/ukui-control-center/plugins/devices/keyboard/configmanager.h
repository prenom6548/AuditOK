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

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "configgroup.h"
#include "fcitxdbusprovider.h"
#include <QObject>
#include <memory>

class ConfigModel : public QObject {
  Q_OBJECT

public:
  Q_PROPERTY(QList<ConfigGroup *> configGroups READ configGroups CONSTANT)

  QList<ConfigGroup *> configGroups() const {
    QList<ConfigGroup *> result;
    for (auto &item : m_configs) {
      result << item.get();
    }
    return result;
  }
  ConfigModel(FcitxDbusProvider *provider, const QString &uri,
              QObject *parent = nullptr);
  ~ConfigModel() override;

  QString joinPath(const QString &path, const QString &option);
  void setupItems(std::unique_ptr<ConfigGroup> &group, const QString &type,
                  const QString &path);
  void addConfigItem(std::unique_ptr<ConfigGroup> &group,
                     const FcitxQtConfigOption &option, const QString &path);
  void setValue(const QVariant &value);
  void getConfig(const bool &sync);
  void restore();
  void save() const;
  const FcitxQtConfigOptionList *optionsForType(const QString &type) const;

signals:
  void configurationChanged();

private slots:
  void requestConfigFinished(QDBusPendingCallWatcher *watcher);

private:
  QVariant generateSaveValue() const;
  FcitxDbusProvider *m_provider = nullptr;
  std::vector<std::unique_ptr<ConfigGroup>> m_configs;
  std::unique_ptr<ConfigGroup> m_configGroup = nullptr;

  QMap<QString, FcitxQtConfigOptionList> m_desc;
  QString m_rootType;
  QString m_uri;
  bool m_initialized = false;
};

class ConfigManager : public QObject {
  Q_OBJECT
public:
  Q_PROPERTY(QList<ConfigGroup *> globalConfig READ globalConfig CONSTANT)
  Q_PROPERTY(QList<ConfigGroup *> imConfig READ imConfig CONSTANT)
  Q_PROPERTY(QList<ConfigGroup *> externalConfig READ externalConfig CONSTANT)
  Q_INVOKABLE void saveImConfig();
  Q_INVOKABLE void reloadImConfig();
  Q_INVOKABLE void restoreDefaultImConfig();
  Q_INVOKABLE void saveGlobalConfig();

  Q_INVOKABLE void restoreDefaultExternalConfig();
  Q_INVOKABLE void saveExternalConfig();

  // 拉起搜狗输入法的配置工具
  // 按现有fcitx5的配置形式，无法打开搜狗配置，所以对搜狗的配置做了单独处理
  Q_INVOKABLE void launchSogouConfigTool(const QString &uniqueName);

  QList<ConfigGroup *> globalConfig() const {
    if (m_gloalConfig) {
      return m_gloalConfig->configGroups();
    } else {
      return {};
    }
  }
  QList<ConfigGroup *> imConfig() const {
    if (m_imConfig) {
      return m_imConfig->configGroups();
    } else {
      return {};
    }
  }
  QList<ConfigGroup *> externalConfig() const {
    if (m_externalConfig) {
      return m_externalConfig->configGroups();
    } else {
      return {};
    }
  }

  ConfigManager(FcitxDbusProvider *provider, QObject *parent = nullptr);

  ~ConfigManager() override = default;

  Q_INVOKABLE void getImConfig(const QString &uniqueName);

  Q_INVOKABLE void getExternalConfig(const QString &uri);

  Q_INVOKABLE void releaseImConfigResource();

  Q_INVOKABLE void releaseExternalConfigResource();

private:
  void getGlobalConfig(const bool &sync);
  FcitxDbusProvider *m_provider = nullptr;
  std::unique_ptr<ConfigModel> m_gloalConfig = nullptr;
  std::unique_ptr<ConfigModel> m_imConfig = nullptr;
  std::unique_ptr<ConfigModel> m_externalConfig = nullptr;
  const QString m_globalConfigUri = "fcitx://config/global";
  const QString m_imConfigUriPrefix = "fcitx://config/inputmethod/";
  bool m_initialized = false;

signals:
  void globalConfigChanged();
  void imConfigurationChanged();
};

#endif // CONFIGMANAGER_H
