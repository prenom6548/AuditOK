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
#ifndef OUTPUTCONFIG_H
#define OUTPUTCONFIG_H

#include <QGroupBox>
#include <QComboBox>
#include <QWidget>

#include <KF5/KScreen/kscreen/output.h>

#include <QGSettings>

#include <QTimer>

#include <QDBusInterface>

#include "kswitchbutton.h"
#include "ukccframe.h"
using namespace kdk;

class QCheckBox;
class ResolutionSlider;
class QLabel;
class QStyledItemDelegate;

namespace Ui {
class KScreenWidget;
}

class OutputConfig : public UkccFrame
{
    Q_OBJECT

public:
    explicit OutputConfig(QWidget *parent);
    explicit OutputConfig(const KScreen::OutputPtr &output, QWidget *parent = nullptr);
    ~OutputConfig() override;

    virtual void setOutput(const KScreen::OutputPtr &output);
    KScreen::OutputPtr output() const;

    void setTitle(const QString &title);
    void setShowScaleOption(bool showScaleOption);
    bool showScaleOption() const;

    void initConfig(const KScreen::ConfigPtr &config);
    void initScaleItem();

    double getGlobalData(const KScreen::OutputPtr &output);
    void setRestore();
    KScreen::ModePtr findBestMode(const QSize &size);
    KScreen::Output::Rotation getRotationStatus();
    bool is9100VgaGpu();

public Q_SLOTS:
    void slotUpdateScale();

protected Q_SLOTS:
    void slotResolutionChanged(const QSize &size, bool emitFlag);
    void slotRotationChanged(int index);
    void slotScaleChanged(int index);
    void rotationDbusSlot(bool autoRotation);
    void tabletChanged(bool tabletMode);
    void rotationDirectionSlot(QString rotationDirection);
    void slotRefreshRateChanged(int index);
    void slotEnableWidget();
    void setEnableBySecurity();

Q_SIGNALS:
    void changed();
    void scaleChanged(QSize size);
    void toSetScreenPos();
    void enabledChanged();

public:
    bool m_is9100 = false;

protected:
    virtual void initUi();
    double getScreenScale();
    QString refreshRateToText(float refreshRate);
    void loadResolutionRefreshRateBlack();
    bool hitResolutionRefreshRateBlack(QString resolution, QString refreshRate);

private:
    void initConnection();
    QString scaleToString(double scale);
    void hideComponent();
    void resolutionChangedSlot(QSize size, bool emitFlag);

protected:
    KScreen::OutputPtr mOutput;
    QLabel *mTitle = nullptr;
    QCheckBox *mEnabled = nullptr;
    ResolutionSlider *mResolution = nullptr;

    QComboBox *mRotation = nullptr;
    KSwitchButton *mAutoRotation = nullptr;
    QComboBox *mRefreshRate = nullptr;
    QComboBox *mMonitor = nullptr;
    QComboBox *mScaleCombox = nullptr;

    UkccFrame    *mRotateFrame    = nullptr;
    UkccFrame *mAutoRotationFrame = nullptr;
    UkccFrame *m_freshFrame = nullptr;
    UkccFrame *m_resFrame = nullptr;
    UkccFrame *m_scaleFrame = nullptr;

    bool mShowScaleOption = false;
    bool mIsWayland = false;
    bool mIsFirstLoad = true;
    bool mIsRestore = true;  //非restore时不再去修改刷新率/分辨率，避免修改多次导致显示重复
    bool needLogout = false;
    bool mSetRestore = false;

    KScreen::ConfigPtr mConfig = nullptr;

    QGSettings *mDpiSettings = nullptr;

    QDBusInterface *mStatusManager = nullptr;
    QMap<QString, QStringList> m_resolutionRefreshRateBlackMap;
    QDBusInterface *m_usdDbus = nullptr;
    bool m_showPrefer = false;
    double m_preferScale = 0;
    double m_preferIndex = -1;
};

#endif // OUTPUTCONFIG_H
