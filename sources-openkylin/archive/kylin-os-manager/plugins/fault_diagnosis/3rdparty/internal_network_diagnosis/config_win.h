/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#ifndef CONFIG_WIN_H
#define CONFIG_WIN_H

#include <QWidget>
#include <QDialog>

#define CUSTOM_NETWORK_SYSTEM_CONFIG_FILE "/etc/kylin-os-manager/net-check/kylin-netcheck-tools.conf"
#define CUSTOM_NETWORK_USER_CONFIG_FILE ".config/kylin-os-manager/net-check/kylin-netcheck-tools.conf"

namespace Ui
{
class ConfigWin;
}

namespace custom_network {

struct Config {
    Config();
    bool ParseConfig(const QString &path);
    bool SaveConfig(const QString &path);

    bool on_;
    QList<QString> ips_;
    QList<QString> webs_;
};

}

class ConfigWin : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWin(QWidget *parent = nullptr);
    ~ConfigWin();

public slots:
    void showWin();
    void resizeWinSize();
    void recordChange();
private slots:
    void setInnerCheckShow(bool isCheck);
    void cancelPress();
    void savePress();
signals:
    void refreshUI();
    void sigUpdateConfigFile();

private:
    Ui::ConfigWin *ui;
    void setWin();
    bool saveSettings();
    bool m_isShow = false;
    bool m_isON = true;

    QString system_config_path_;
    QString user_config_path_;
    custom_network::Config system_config_;
    custom_network::Config user_config_;
};

#endif
