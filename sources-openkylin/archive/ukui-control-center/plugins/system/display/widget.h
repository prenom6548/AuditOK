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
#ifndef WIDGET_H
#define WIDGET_H

#include <limits>

#include <QMutex>
#include <QWidget>
#include <QVariantMap>
#include <QGSettings>
#include <QSettings>
#include <QButtonGroup>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QStringList>
#include <QSharedPointer>
#include <QHash>
#include <QVariant>
#include <QScreen>
#include <QMultiMap>
#include <QShortcut>
#include <QFuture>
#include <QVBoxLayout>
#include <QCheckBox>
#include "splicedialog.h"
#include <KF5/KScreen/kscreen/config.h>

#include "outputconfig.h"
#include "brightnessFrame.h"
#include "lightlabel.h"
#include "switchwidget.h"
#include "kswitchbutton.h"
#include "ukccframe.h"
#include "settinggroup.h"
#include "comboxwidget.h"
#include "sliderwidget.h"
#include <QRadioButton>
#include <klabel.h>

using namespace kdk;

#define USD_CLONE_MODE 1

class QLabel;
class QMLOutput;
class QMLScreen;
class ControlPanel;
class PrimaryOutputCombo;

class QPushButton;
class QComboBox;

class QQuickView;
class QQuickWidget;
class QStyledItemDelegate;

typedef enum {
    SUN,
    CUSTOM,
}MODE;

typedef enum {
#ifndef MAVIS
    FIRST,
    VICE,
#endif
    EXTEND,
    CLONE,
    SPLICE
}MULTISCREEN;

namespace KScreen {
class ConfigOperation;
}

namespace Ui {
class DisplayWindow;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void setConfig(const KScreen::ConfigPtr &config);
    KScreen::ConfigPtr currentConfig() const;

    void initNightModeUi();
    void initNightModeConnection();
    // OutputConfig 缩放创建之后再建开关，Tab：缩放 → 打开显示器
    void initOpenMonitorButton();
    void alignScaleToOpenMonitorTabOrder();
    void slotFocusedOutputChangedNoParam();
    void initConnection();
    QString getScreenName(QString name = "");
    void setNightComponent();

    bool writeFile(const QString &filePath);
    void writeGlobal(const KScreen::OutputPtr &output);
    bool writeGlobalPart(const KScreen::OutputPtr &output, QVariantMap &info,
                         const KScreen::OutputPtr &fallback);
    QString globalFileName(const QString &hash);
    QVariantMap getGlobalData(KScreen::OutputPtr output);

    float converToScale(const int value);
    int scaleToSlider(const float value);

    void initUiComponent();
    void hideComponent();
    void addBrightnessFrame(QString name, bool openFlag, QString edidHash, const KScreen::OutputPtr &output);
    void showBrightnessFrame(const int flag = 0);
    void changescale(QSize size = QSize());
    void getAllI2Cbus();
    void getEdidI2Cbus();
    void initUi();
    void initEnhanceBrightUI();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void showEvent(QShowEvent *event) override;

Q_SIGNALS:
    void changed();

    void nightModeChanged(const bool nightMode) const;
    void redShiftValidChanged(const bool isValid) const;

private Q_SLOTS:
    void slotFocusedOutputChanged(QMLOutput *output);

    void slotOutputEnabledChanged();
    void slotOutputConnectedChanged();

    void slotUnifyOutputs();

    void slotIdentifyButtonClicked(bool checked = true);
    void slotIdentifyOutputs(KScreen::ConfigOperation *op);
    void clearOutputIdentifiers();

    void outputAdded(const KScreen::OutputPtr &output, bool connectChanged);
    void outputRemoved(int outputId, bool connectChanged);
    void primaryOutputSelected(int index);
    void primaryOutputChanged(const KScreen::OutputPtr &output);
    void primaryButtonEnable(bool);             // 按钮选择主屏确认按钮
    void mainScreenButtonSelect(int index);     // 是否禁用设置主屏按钮
    void checkOutputScreen(bool checked);         // 是否禁用屏幕

    void setNightMode(const bool nightMode);    // 设置夜间模式

    void initNightStatus();                     // 初始化夜间模式

    void setNightModeSetting();                 // 通过配置文件设置夜间模式
    QString getSunTime();                          // 获取日出日落时间

    void callMethod(QRect geometry, QString name);// 设置wayland主屏幕
    QString getPrimaryWaylandScreen();
    void usdScreenModeChangedSlot(int status);

    void setMultiScreenSlot(const QString &screenMode);

    void applyNightModeSlot();
    void tempSliderChangedSlot(int tempValue);
    void tempSliderReleasedSlot();

    void delayApply();

public Q_SLOTS:
    void save();
    void changedSlot();
    void propertiesChangedSlot(QString, QMap<QString, QVariant>, QStringList);
    void mOutputClicked();
    void scaleChangedSlot(double scale);

private:
    void loadQml();
    void resetPrimaryCombo();
    void addOutputToPrimaryCombo(const KScreen::OutputPtr &output);
    KScreen::OutputPtr findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info);
    // 进页只绑读屏名，不抢侧栏焦点；预览黑框由用户 Tab 进入时出现
    void focusPreviewKeyboardFrame();
    // Preview 读屏文案（译文「预览图」；兼容旧 qm 的「预览」）
    QString previewAccessibleLabel() const;
    // 按从左到右收集可 Tab 的预览块
    QList<QMLOutput *> tabbablePreviewOutputs() const;
    // 末块 Tab→插件内下一控件；首块 Shift+Tab→插件外上一控件（侧栏）
    bool leavePreviewOnTabEdge(bool forward);
    // QML 焦点对 Orca 不可见：把当前预览块名称同步到 QQuickWidget
    void bindPreviewAccessibleNames();
    void syncPreviewAccessibleName(QMLOutput *output);
    void onPreviewOutputActiveFocusChanged();

    void initComponent();
    void initDbusComponent();
    void setHideModuleInfo();
    void writeScale(double scale);
    void initGSettings();
    void setcomBoxScale();
    void initAutoBrihgtUI();
    void initAdvanceScreen();

    bool isRestoreConfig();
    KScreen::ConfigPtr remapRestoreConfigByName(const KScreen::ConfigPtr &prevConfig) const;
    bool isCloneMode();
    bool isBacklight();
    bool isPanguX();
    bool isCe72();
    bool isBacklightAllInOne();
    bool isLaptopScreen();
    bool isVisibleBrightness();
    bool isSetGammaBrightness();
    bool isSupoortGammaBrighnes();

    QString getCpuInfo();
    QString getMonitorType();

    int getPrimaryScreenID();

    void setScreenIsApply(bool isApply);

    void setMulScreenVisiable();
    void initMultScreenStatus(int status = -1);
    void updateMultiScreen(); // 屏幕插拔更改状态
    int updateScreenConfig(); // 获取系统最新配置

    void showZoomtips();      // 缩放注销提示框
    void compareScale();
    void outputEnableChanged();
    void outputSetDarkMode(QGSettings* styleSettings, const QString &key);
    QFrame *setLine(QFrame *frame);
    void primaryCheckBoxClickSlot(bool checked);
    void eyesModeFrameSlot(bool checked);
    void colorSettingChangedSlot(const QString &key);
    void openTimeHComboxIdxChangeSlot();
    void openTimeMComboxIdxChangeSlot();
    void closeTimeHComboxIdxChangeSlot();
    void closeTimeMComboxIdxChangeSlot();
    void timeModeComboBoxIdxChangeSlot();
    void customRadioButtonClickSlot();
    void defaultRadioButtonClickSlot();
    void setEnableBySecurity();
    int getConnectedOutputNum();
    void powerGsettingChangedSlot(const QString &key);
    void enhanceBrightBtnChanged(bool status);

private:
    Ui::DisplayWindow *ui;
    QMLScreen *mScreen = nullptr;

    KScreen::ConfigPtr mConfig     = nullptr;
    KScreen::ConfigPtr mPrevConfig = nullptr;
    KScreen::OutputPtr res         = nullptr;
    OutputConfig *mOutputConfig    = nullptr; // 设置主显示器相关控件

    QList<QQuickView *> mOutputIdentifiers;
    QTimer *mOutputTimer           = nullptr;

    QString mCPU;
    QString mDir;
    QString mScaleDir;
    QString mUsdScaleDir;
    QStringList mPowerKeys;

    KSwitchButton *mUnifyButton   = nullptr;
    KSwitchButton *mAutoBrightBtn = nullptr;
    KSwitchButton *mBackBrightBtn = nullptr;
    KSwitchButton *m_enhanceBrightBtn = nullptr;

    KLabel *m_nightModeLabel = nullptr;

    KLabel *m_customTimeLabel    = nullptr;
    KLabel *mTemptLabel       = nullptr;
    KLabel *mWarmLabel        = nullptr;
    KLabel *mColdLabel        = nullptr;
    KLabel *m_toLabel           = nullptr;
    KLabel* m_enhanceBrightLabel = nullptr;

    KLabel *mMultiScreenLabel   = nullptr;
    LightLabel *mBackHintLabel  = nullptr;
    LightLabel *mBrightHintLabel= nullptr;
    LightLabel *mNightHintLabel = nullptr;

    QComboBox *m_openTimeHCombox   = nullptr;
    QComboBox *m_openTimeMCombox   = nullptr;
    QComboBox *m_closeTimeHCombox  = nullptr;
    QComboBox *m_closeTimeMCombox  = nullptr;
    QComboBox *mMultiScreenCombox = nullptr;

    QGSettings *scaleGSettings      = nullptr;
    QGSettings *mPowerGSettings     = nullptr;
    QGSettings *m_colorSettings     = nullptr;
    QGSettings *mAutoBrightSettings = nullptr;

    QGSettings* m_gsettings         = nullptr;
    QSettings *mQsettings = nullptr;

    QButtonGroup *singleButton;

    QSharedPointer<QDBusInterface> mUPowerInterface;
    QSharedPointer<QDBusInterface> mUkccInterface;
    QDBusInterface *mUsdDbus;
    QDBusInterface *m_StatusDbus = nullptr;

    QHash<QString, QVariant> mNightConfig;

    double mScreenScale = 1.0;
    double scaleres = 1.0;
    QMap <QString, double> firstChangeScaleList;
    double firstChangeScale = 0.0;

    QSize mScaleSizeRes = QSize();

    bool m_isNightMode     = false;
    bool mIsScaleChanged  = false;
    bool mOriApply;
    bool mConfigChanged   = false;
    bool mOnBattery       = false;
    bool mBlockChanges    = false;
    bool mFirstLoad       = true;
    bool m_nightModeUiInited = false;
    bool mIsWayland       = false;
    bool mIsBattery       = false;
    bool mIsScreenAdd     = false;
    bool mIsRestore       = false;

    bool mIsSCaleRes      = false;
    bool mIsChange        = false;
    bool exitFlag         = false;
    bool unifySetconfig   = false;
    bool mIscloneMode     = false;
    bool m_isEyeCareMode   = false;
    bool m_isPanguX        = false;
    bool m_isCe72         = false;
    bool m_isSetGM        = false;
    bool m_isSupportGM    = false;

    QStringList mTimeModeStringList;

    QString firstAddOutputName;
    QShortcut *mApplyShortcut;
    QVector<BrightnessFrame*> BrightnessFrameV;
    QString mOutputClickedName;
    QDBusInterface *dbusEdid = nullptr;
    QString cpuArchitecture = "";
    QString mMachineType;

    int mKdsStatus = INT_MIN;
    int slotTimeDelay = 2500;
    QMap<QString, QString> I2CbusMap;
    QMap<QString, QString> edidI2CbusMap;

    int connectedOutputNum;
    QLabel *spliceMethodLabel = nullptr;
    int m_temptSliderValue = 0;

private:
    SettingGroup  *modeGroup         = nullptr;
    UkccFrame     *mMultiScreenFrame = nullptr;
    SettingGroup  *configGroup       = nullptr;
    UkccFrame     *monitorFrame      = nullptr;
    KLabel      *monitorLabel      = nullptr;
    QComboBox     *monitorComboBox   = nullptr;
    QPushButton   *monitorBtn        = nullptr;
    ControlPanel  *mControlPanel     = nullptr;
    UkccFrame     *scaleFrame        = nullptr;
    KLabel      *scaleLabel        = nullptr;
    QComboBox     *scaleComboBox     = nullptr;
    UkccFrame     *openMonitorFrame  = nullptr;
    KLabel      *openMonitorLabel  = nullptr;
    KSwitchButton *openMonitorButton = nullptr;
    UkccFrame     *mbrightnessFrame  = nullptr;
    QVBoxLayout   *mbrightnesslayout = nullptr;
    SettingGroup  *m_nightModeFrame   = nullptr;
    UkccFrame* m_defaultNightModeFrame = nullptr;
    UkccFrame* m_customNightModeFrame = nullptr;
    UkccFrame* m_tempFrame = nullptr;
    UkccFrame* m_timeModeFrame = nullptr;
    UkccFrame* m_customTimeFrame = nullptr;
    QRadioButton* m_defaultRadioButton = nullptr;
    QRadioButton* m_customRadioButton = nullptr;
    KSlider* m_tempSlider = nullptr;
    QComboBox* m_timeModeComboBox = nullptr;
    SwitchWidget* m_eyesModeSwitchWidget = nullptr;

    SwitchWidget *mAutoBrightFrame = nullptr;
    QCheckBox* m_primaryCheckBox = nullptr;
    SettingGroup* m_primaryGroup = nullptr;
    UkccFrame* m_primaryFrame = nullptr;
    QFrame* m_primaryLine = nullptr;
    bool m_firstViewResize = false;
    QMap<int, QString> m_outputIdToNameMap;
    QFrame *m_enhanceBrightFrame = nullptr;
    bool m_enhanceBrightOpen = false;
};

#endif // WIDGET_H
