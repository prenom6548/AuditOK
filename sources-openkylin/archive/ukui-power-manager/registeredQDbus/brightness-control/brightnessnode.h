/*
 * Copyright 2023 KylinSoft Co., Ltd.
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

#ifndef BRIGHTNESSNODE_H
#define BRIGHTNESSNODE_H

#include <QObject>
#include <QTimer>

#include "../upmcustomhardwareconfig.h"

#define BRIGHTNESS_NODE_PATH "/sys/class/backlight/"

class BrightnessNode : public QObject
{
    Q_OBJECT
public:
    explicit BrightnessNode(QObject *parent = nullptr);
    ~BrightnessNode();

private:
    bool m_hasEffectiveNode;
    int m_maxbrightness;
    bool m_isCanAdjustBright;
    bool m_isBrightnessEnhancementSupported;
    QStringList m_nodeDirs;
    QString m_effectiveNodePath;
    QTimer *m_BrightnessTimer = nullptr;
    QTimer *m_brightnessAdjustTimer = nullptr;
    int m_targetBrightness = 0;
    int m_currentBrightness = 0;
    int m_brightnessStep = 1; // 每次调整的步长

    void initBrightnessNodeInfo(QString &brightnessNode);
    QString getBrightnessNodeFileContent(const QString &strNodePath, const QString &strNodeFile);
    bool isBrightnessAdjustable();
    void initBrightnessEnhancementCapability();
    void setSmootBrightness();
public:
    bool canSetBrightness();
    int getMaxBrightness();
    int getCurrentBrightness();
    bool isBrightnessEnhancementSupported();
    QString setCurrentBrightness(int iBrightness);
private Q_SLOTS:
    void onBrightnessTimeOut();
};

#endif // BRIGHTNESSNODE_H
