/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: sundagao <sundagao@kylinos.cn>
 */

#ifndef CONTROLCONFIG_H
#define CONTROLCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <optional>
namespace UkuiNotification {

class ControlConfig : public QObject {
    Q_OBJECT
public:
    enum ControlMode {
        None = 0,       // 不启用
        Whitelist = 1, // 白名单
        Blacklist = 2, // 黑名单
        Invalid   // 无效
    };

    explicit ControlConfig(const QString &configName, const QJsonObject &config, QObject *parent = nullptr);
    std::optional<bool> isAppAllowed(const QString &desktopEntry) const;
    bool isOperationAllowed(const QString &option) const;
    bool isEnabled() const;
    void updateConfig(const QJsonObject &config);
    QStringList appList() const;

    static inline bool isValidMode(const int mode) {
        return mode >= static_cast<int>(ControlMode::None)
                && mode < static_cast<int>(ControlMode::Invalid);
    }

private:
    void parseConfig(const QJsonObject &config);

    QString m_configName;
    ControlMode m_mode = ControlMode::None;
    QStringList m_appList;
};

} // UkuiNotification

#endif //CONTROLCONFIG_H
