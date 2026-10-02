/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef ConfigurationManager_H
#define ConfigurationManager_H

#include <QObject>
#include <QDebug>


class QSettings;
class QString;
class ConfigurationManager : public QObject
{
    Q_OBJECT
public:
    static ConfigurationManager *GetInstance();
    QString getDeviceOrientation() const;
    void setDeviceOrientation(QString &orientation);
    bool isTabletMode() const;
    void setTabletMode(bool tabletMode);
    bool isTabletModeEnabled() const;
    void setTabletModeEnabled(bool enabled);
    bool isAutoSwitchEnabled() const;
    void setAutoSwitchEnabled(bool autoSwitchEnable);
    bool isAutoRotateEnabled() const;
    void setAutoRotateEnabled(bool autoRotateEnable);
    bool isFirstSwitchDeviceMode() const;
    void setFirstSwitchDeviceModeToOff();
    //Immersive Mode
    QMap<QString, QVariant> liteMode() const;
    /**
     * @brief isclassicalMode
     * @return 0:三岛模式，1经典模式。
     */
    bool isClassicalMode() const;
    void setClassicalMode(bool classicalMode);

    /**
     * @brief isImmersiveMode
     * @return 0非沉浸模式，1沉浸模式
     */
    bool isImmersiveMode() const;

    /**
     * @brief setImmersiveMode
     * @param ImmersiveMode 0:非沉浸模式，1：沉浸模式
     */
    void setImmersiveMode(bool ImmersiveMode);
private:
    ConfigurationManager();
    ConfigurationManager(const ConfigurationManager &) = delete;
    const ConfigurationManager &operator =(const ConfigurationManager &) = delete;
    void createConfigFile();

private:
    QSettings *m_qsetting = nullptr;
};

#endif
