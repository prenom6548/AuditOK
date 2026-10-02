/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef UNIFIEDOUTPUTCONFIG_H
#define UNIFIEDOUTPUTCONFIG_H

#include "outputconfig.h"

namespace KScreen {
class Output;
class Config;
}

class UnifiedOutputConfig : public OutputConfig
{
    Q_OBJECT
public:
    explicit UnifiedOutputConfig(const KScreen::ConfigPtr &config, QWidget *parent);
    ~UnifiedOutputConfig() override;

    void setOutput(const KScreen::OutputPtr &output) override;
    bool isCloneMode();
    void initScaleItem();

    double getGlobalData(KScreen::OutputPtr output);
private Q_SLOTS:
    void slotResolutionChanged(const QSize &size, bool emitFlag);

    // 统一输出后调整屏幕方向统一代码
    void slotRotationChangedDerived(int index);
    void rotationDbusSlot(bool autoRotation);
    void tabletChanged(bool tabletMode);
    void rotationDirectionSlot(QString rotationDirection);
    void slotRestoreResoltion();
    void slotRestoreRatation();
    void slotRefreshRateChanged(int index);
    void slotScaleChanged(int index);
    void setEnableBySecurity();

private:
    void initUi() override;
    KScreen::OutputPtr createFakeOutput();
    QString findBestMode(const KScreen::OutputPtr &output, const QSize &size);
    QFrame *setLine(QFrame *frame);
    void resolutionChangedSlot(QSize size, bool emitFlag);

private:
    KScreen::ConfigPtr mConfig;
    QList<KScreen::OutputPtr> mClones;
    bool mIsRestore;
    bool needLogout = false;
    QMap <QString, double> mScaleList;
    QDBusInterface *mStatusManager = nullptr;
    QDBusInterface *m_usdDbus = nullptr;
    bool m_showPrefer = false;
    double m_preferScale = 0;
    double m_preferIndex = -1;
    QWidget *m_freshLineFrame = nullptr;

Q_SIGNALS:
    void scaleChanged(QSize size);
};

#endif // UNIFIEDOUTPUTCONFIG_H
