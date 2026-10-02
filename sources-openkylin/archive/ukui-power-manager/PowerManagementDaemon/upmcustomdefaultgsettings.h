/*
 * Copyright 2024 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UPMCUSTOMDEFAULTGSETTINGS_H
#define UPMCUSTOMDEFAULTGSETTINGS_H

#include <QObject>
#include <QGSettings>
#include <QSettings>

#define UPM_NO_THIS_KEY    QStringLiteral("no-this-key")

#define UPM_HARDWARE_INFO_FILE            QStringLiteral("/sys/class/dmi/id/modalias")
#define UPM_GLOBAL_GSETTINGS_CONFIG_FILE  QStringLiteral("/usr/share/ukui/ukui-power-manager/upm-gsettings-global.conf")
#define UPM_EXTEND_GSETTINGS_CONFIG_PATH  QStringLiteral("/usr/share/ukui/ukui-power-manager/upm-gsettings-extend.d/")

#define UPM_GROUP_DEFAULT_POLICY_AC          QStringLiteral("default-policy-ac")
#define UPM_GROUP_DEFAULT_POLICY_BATTERY     QStringLiteral("default-policy-battery")

#define UPM_KEY_BALANCE_MODE_BOARD_NAME      QStringLiteral("balanceModeBoardName")
#define UPM_KEY_POWERSAVE_MODE_BOARD_NAME    QStringLiteral("powersaveModeBoardName")

#define UPM_GROUP_EXTEND_BRAND_GENERIC_CONFIG   QStringLiteral("Brand-generic-config")

class UpmCustomDefaultGsettings : public QObject
{
    Q_OBJECT
public:
    explicit UpmCustomDefaultGsettings(QObject *parent = nullptr);
    ~UpmCustomDefaultGsettings();

    void setUpmCustomDefaultGsettings();

private:
    QString m_hardwareInfo = "";
    QSettings *m_globalConfig = nullptr;
    QSettings *m_extendConfig = nullptr;
    QString m_productGroup = "";
    QGSettings *m_upmGsettings = nullptr;

    void getHardwareInfo();

    void setCustomDefaultCpuPolicyAc();
    void setCustomDefaultCpuPolicyBattery();
    void setCustomDefaultIdleBrightness();
    void setCustomDefaultIdlePowerPolicyAuto();

    void setCustomDefaultSleepComputerAc();
    void setCustomDefaultSleepComputerBattery();

    void setCustomDefaultIdleCloseDisplayTime();

    bool getGlobalCpuDefaultPolicy(const QString &group, const QString &key);

    bool readConfig(QSettings *settings, const QString &group,
                    const QString &key, QVariant &value);
    bool getExtendConfigFile(QString &configFilePath);
    bool readAll(const QString &filePath, QString &content);

signals:

};

#endif // UPMCUSTOMDEFAULTGSETTINGS_H
