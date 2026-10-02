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
#include "widget.h"
#include "controlpanel.h"
#include "declarative/qmloutput.h"
#include "declarative/qmlscreen.h"
#include "utils.h"
#include "ui_display.h"
#include "colorinfo.h"
#include "scalesize.h"
#include "ukcccommon.h"
using namespace ukcc;

#include <QHBoxLayout>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QSplitter>
#include <QShowEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QAccessible>
#include <QLocale>
#include <algorithm>
#include <QtGlobal>
#include <QQuickView>
#include <qquickitem.h>
#include <QDebug>
#include <QPushButton>
#include <QProcess>
#include <QtAlgorithms>
#include <QtXml>
#include <QDomDocument>
#include <QDir>
#include <QStandardPaths>
#include <QComboBox>
#include <QQuickWidget>
#include <QMessageBox>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QtConcurrent>
#include <QVariantMap>
#include <QDBusMetaType>
#include <unistd.h>
#include <QListView>
#include <QStandardItem>

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/mode.h>
#include <KF5/KScreen/kscreen/config.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/setconfigoperation.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/types.h>

#include <math.h>
#include <kysdk/kysdk-system/libkysysinfo.h>
#include <themeController.h>
#include <kysdk/applications/accessinfohelper.h>
#include "windowmanager/windowmanager.h"
using namespace kdk;

#ifdef signals
#undef signals
#endif

#define UKUI_CONTORLCENTER_PANEL_SCHEMAS "org.ukui.control-center.panel.plugins"
#define THEME_NIGHT_KEY                  "themebynight"

#define FONT_RENDERING_DPI               "org.ukui.SettingsDaemon.plugins.xsettings"
#define SCALE_KEY                        "scaling-factor"

#define USD_BRIGHT_SCHEMAS               "org.ukui.SettingsDaemon.plugins.auto-brightness"
#define BRIGHT_KEY                       "auto-brightness"
#define DYNAMIC_BRIHT_KEY                "dynamic-brightness"

#define SETTINGS_DAEMON_COLOR_SCHEMAS    "org.ukui.SettingsDaemon.plugins.color"
#define AUTO_KEY                         "night-light-schedule-automatic"
#define AllDAY_KEY                       "night-light-allday"
#define NIGHT_ENABLE_KEY                 "night-light-enabled"
#define NIGHT_FROM_KEY                   "night-light-schedule-from"
#define NIGHT_TO_KEY                     "night-light-schedule-to"
#define AUTO_NIGHT_FROM_KEY              "night-light-schedule-automatic-from"
#define AUTO_NIGHT_TO_KEY                "night-light-schedule-automatic-to"
#define NIGHT_TEMPERATURE_KEY            "night-light-temperature"
#define EYES_MODE                        "eye-care"
#define MAIN_SWITCH                      "main-switch"

#define MOUSE_SIZE_SCHEMAS               "org.ukui.peripherals-mouse"
#define CURSOR_SIZE_KEY                  "cursor-size"

#define POWER_SCHMES                     "org.ukui.power-manager"
#define POWER_KEY                        "brightness-ac"
#define BRIGHTNESS_ENHANCE_SWITCH_KEY    "brightness-enhancement-switch"

#define ADVANCED_SCHEMAS                 "org.ukui.session.required-components"
#define ADVANCED_KEY                     "windowmanager"

const QString kCpu = "ZHAOXIN";
const QString kLoong = "Loongson";
const QString tempDayBrig = "6500";

#define UKUI_STYLE_ID                    "org.ukui.style"
#define UKUI_STYLE_NAME                  "style-name"

void value_to_hour_minute(double value, int *hour, int *minute)
{
    double hours;
    double mins = 0.f;
    /* display the right thing for AM/PM */
    mins = modf (value, &hours) * 60.f;
    *hour = int(hours);
    *minute = int(mins);
}

double hour_minute_to_value(int hour, int minute) {
    double value = (double)minute/60;
    return (double)hour + value;
}

Q_DECLARE_METATYPE(KScreen::OutputPtr)

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DisplayWindow())
{
    cpuArchitecture = UkccCommon::getCpuArchitecture();
    qRegisterMetaType<QQuickView *>();

    ui->setupUi(this);
    ui->titleLabel->setContentsMargins(16, 0, 0, 0);
    initUi();
    initAutoBrihgtUI();
    initEnhanceBrightUI();
    initDbusComponent();
    ui->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    ui->quickWidget->setContentsMargins(0, 0, 0, 9);
    // 预览需进 Tab 链；默认焦点策略往往跳过 QQuickWidget
    ui->quickWidget->setFocusPolicy(Qt::StrongFocus);
    ui->quickWidget->setAccessibleName(previewAccessibleLabel());
    // Tab 在末块预览后交给下方控件，避免只在 QML 内循环
    ui->quickWidget->installEventFilter(this);
    firstAddOutputName = "";
    connectedOutputNum = 0;
    m_outputIdToNameMap.clear();
    initComponent();
    setHideModuleInfo();

    initGSettings();
    initUiComponent();

    initConnection();
    loadQml();

    hideComponent();

    mScreenScale = scaleGSettings->get(SCALE_KEY).toDouble();
}

Widget::~Widget()
{
    for (BrightnessFrame *m_frame : BrightnessFrameV) {
        if (m_frame) {
            delete m_frame;
            m_frame = nullptr;
        }
    }
    if (m_defaultNightModeFrame) {
        delete m_defaultNightModeFrame;
        m_defaultNightModeFrame = nullptr;
    }
    if (m_customNightModeFrame) {
        delete m_customNightModeFrame;
        m_customNightModeFrame = nullptr;
    }
    if (m_tempFrame) {
        delete m_tempFrame;
        m_tempFrame = nullptr;
    }
    if (m_timeModeFrame) {
        delete m_timeModeFrame;
        m_timeModeFrame = nullptr;
    }
    if (m_customTimeFrame) {
        delete m_customTimeFrame;
        m_customTimeFrame = nullptr;
    }
    clearOutputIdentifiers();
    m_outputIdToNameMap.clear();
    delete ui;
    ui = nullptr;
}

void Widget::setEnableBySecurity()
{
    // 深信服控件禁用
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value("displayEnable").toString();
    QStringList setItems = moduleSettings.split(",");

    qDebug() << Q_FUNC_INFO << moduleSettings;
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        qDebug() << "set item Name: " << item.at(0);
        if (item.at(0) == "mMultiScreenFrame") {
            mMultiScreenCombox->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "mScreenFrame") {
            m_primaryGroup->setEnabled(item.at(1) == "true");
            m_primaryLine->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "scaleFrame") {
            scaleFrame->setEnabled(item.at(1) == "true");
        }

        QTimer::singleShot(1000, [=]() {
            if (item.at(0) == "BrightnessFrameV") {
                // 由于亮度Frame在创建对象后才new
                for (BrightnessFrame *m_frame : BrightnessFrameV) {
                    if (m_frame) {
                        m_frame->setEnabled(item.at(1) == "true");
                    }
                }

            }

            if (item.at(0) == "showMonitorframe") {
                monitorFrame->setEnabled(item.at(1) == "true");
            }
        });

        if (item.at(0) == "mEyesModeFrame" && m_eyesModeSwitchWidget) {
            m_eyesModeSwitchWidget->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "m_openFrame" && m_eyesModeSwitchWidget) {
            m_eyesModeSwitchWidget->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "m_timeModeFrame" && m_timeModeFrame) {
            m_timeModeFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "m_customTimeFrame" && m_customTimeFrame) {
            m_customTimeFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "m_temptFrame") {
            m_tempFrame->setEnabled(item.at(1) == "true");
        }
    }
}

bool Widget::eventFilter(QObject *object, QEvent *event)
{
    if (ui && object == ui->quickWidget) {
        if (event->type() == QEvent::KeyPress
                || event->type() == QEvent::ShortcutOverride) {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            const bool shiftTab = (keyEvent->key() == Qt::Key_Backtab)
                    || (keyEvent->key() == Qt::Key_Tab
                        && keyEvent->modifiers().testFlag(Qt::ShiftModifier));
            const bool plainTab = (keyEvent->key() == Qt::Key_Tab) && !shiftTab;
            if (plainTab || shiftTab) {
                const QList<QMLOutput *> outputs = tabbablePreviewOutputs();
                QMLOutput *focused = nullptr;
                for (QMLOutput *output : outputs) {
                    if (output->hasActiveFocus()) {
                        focused = output;
                        break;
                    }
                }
                if (focused && !outputs.isEmpty()) {
                    if (plainTab && focused == outputs.last()) {
                        if (event->type() == QEvent::ShortcutOverride) {
                            keyEvent->accept();
                            return true;
                        }
                        return leavePreviewOnTabEdge(true);
                    }
                    if (shiftTab && focused == outputs.first()) {
                        if (event->type() == QEvent::ShortcutOverride) {
                            keyEvent->accept();
                            return true;
                        }
                        return leavePreviewOnTabEdge(false);
                    }
                }
            }
        } else if (event->type() == QEvent::FocusIn) {
            // 从上方 Tab 进预览→第一块；从下方 Shift+Tab 进→最后一块
            auto *focusEvent = static_cast<QFocusEvent *>(event);
            const Qt::FocusReason reason = focusEvent->reason();
            if (reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason) {
                QTimer::singleShot(0, this, [this, reason]() {
                    if (!ui || !ui->quickWidget || !mScreen || !ui->quickWidget->hasFocus()) {
                        return;
                    }
                    const QList<QMLOutput *> outputs = tabbablePreviewOutputs();
                    if (outputs.isEmpty()) {
                        return;
                    }
                    QMLOutput *target = (reason == Qt::BacktabFocusReason)
                            ? outputs.last() : outputs.first();
                    target->forceActiveFocus(Qt::TabFocusReason);
                });
            }
        }
    }
    if (event->type() == QEvent::Resize && m_firstViewResize) {
        m_firstViewResize = false;
        if (mOutputIdentifiers.contains(qobject_cast<QQuickView *>(object))) {
            QResizeEvent *e = static_cast<QResizeEvent *>(event);
            const QRect screenSize = object->property("screenSize").toRect();
            QRect geometry(QPoint(0, 0), e->size());
            geometry.moveCenter(screenSize.center());
            static_cast<QQuickView *>(object)->setGeometry(geometry);
            // Pass the event further
        }
    }
    return QObject::eventFilter(object, event);
}

void Widget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 进页勿抢焦：侧栏点击/回车后焦点应留在侧栏「显示器」；预览黑框由用户 Tab 进入时再出现
    bindPreviewAccessibleNames();
}

QList<QMLOutput *> Widget::tabbablePreviewOutputs() const
{
    QList<QMLOutput *> outputs;
    if (!mScreen) {
        return outputs;
    }
    for (QMLOutput *output : mScreen->outputs()) {
        if (!output || !output->isVisible() || !output->activeFocusOnTab()) {
            continue;
        }
        if (output->output() && !output->output()->isConnected()) {
            continue;
        }
        outputs.append(output);
    }
    std::sort(outputs.begin(), outputs.end(), [](QMLOutput *a, QMLOutput *b) {
        if (!qFuzzyCompare(a->x(), b->x())) {
            return a->x() < b->x();
        }
        return a->y() < b->y();
    });
    return outputs;
}

bool Widget::leavePreviewOnTabEdge(bool forward)
{
    if (!ui || !ui->quickWidget) {
        return false;
    }
    // 焦点链环绕：若 Backward 仍只认插件内控件，会落到护眼「关闭时间」等末项，回不到侧栏
    QWidget *const start = ui->quickWidget;
    QWidget *candidate = forward ? start->nextInFocusChain()
                                 : start->previousInFocusChain();
    while (candidate && candidate != start) {
        const bool canTab = candidate->isVisible()
                && candidate->isEnabled()
                && (candidate->focusPolicy() & Qt::TabFocus);
        if (canTab) {
            if (forward) {
                // Tab 出末块预览：落到本插件内下一控件（如设为主显示器）
                if (!isAncestorOf(candidate)) {
                    candidate = candidate->nextInFocusChain();
                    continue;
                }
            } else {
                // Shift+Tab 出首块预览：落到插件外上一控件（侧栏项），跳过页内环绕
                if (isAncestorOf(candidate)) {
                    candidate = candidate->previousInFocusChain();
                    continue;
                }
            }
            if (QQuickItem *root = ui->quickWidget->rootObject()) {
                root->setFocus(false);
            }
            // 先移走焦点再恢复容器名，避免 NameChanged 在仍聚焦时读出裸「预览图」
            candidate->setFocus(forward ? Qt::TabFocusReason : Qt::BacktabFocusReason);
            ui->quickWidget->setAccessibleName(previewAccessibleLabel());
            return true;
        }
        candidate = forward ? candidate->nextInFocusChain()
                            : candidate->previousInFocusChain();
    }
    return false;
}

QString Widget::previewAccessibleLabel() const
{
    QString label = tr("Preview");
    // zh_CN 已改为「预览图」；系统仍装旧 qm（「预览」）时读屏统一用「预览图」
    if (QLocale().name().startsWith(QLatin1String("zh"))
            && (label == QLatin1String("Preview") || label == QStringLiteral("预览"))) {
        label = QStringLiteral("预览图");
    }
    return label;
}

void Widget::focusPreviewKeyboardFrame()
{
    // 仅绑定读屏名；不再 setFocus，避免侧栏进页时抢走「显示器」项焦点
    bindPreviewAccessibleNames();
}

void Widget::bindPreviewAccessibleNames()
{
    if (!mScreen) {
        return;
    }
    for (QMLOutput *output : mScreen->outputs()) {
        if (!output) {
            continue;
        }
        // UniqueConnection：反复 setConfig/进页时避免重复连接
        connect(output, &QQuickItem::activeFocusChanged,
                this, &Widget::onPreviewOutputActiveFocusChanged,
                Qt::UniqueConnection);
    }
}

void Widget::onPreviewOutputActiveFocusChanged()
{
    auto *output = qobject_cast<QMLOutput *>(sender());
    if (!output || !output->hasActiveFocus()) {
        return;
    }
    syncPreviewAccessibleName(output);
}

void Widget::syncPreviewAccessibleName(QMLOutput *output)
{
    if (!ui || !ui->quickWidget || !output) {
        return;
    }
    QString name;
    if (output->isCloneMode()) {
        name = tr("Unity");
    } else {
        name = output->friendlyOutputName();
        if (name.isEmpty() && output->output()) {
            name = output->output()->name();
        }
    }
    // 读屏：预览图 eDP-1（Preview 译文为「预览图」）
    if (name.isEmpty()) {
        ui->quickWidget->setAccessibleName(previewAccessibleLabel());
    } else {
        ui->quickWidget->setAccessibleName(previewAccessibleLabel() + QLatin1Char(' ') + name);
    }
    // QML 内 Tab 不切换 QWidget 焦点，需主动通知名称与焦点变化供 Orca 朗读
    QAccessibleEvent nameEvent(ui->quickWidget, QAccessible::NameChanged);
    QAccessible::updateAccessibility(&nameEvent);
    QAccessibleEvent focusEvent(ui->quickWidget, QAccessible::Focus);
    QAccessible::updateAccessibility(&focusEvent);
}

void Widget::outputSetDarkMode(QGSettings* styleSettings, const QString &key) {
    if (nullptr == styleSettings || key != "styleName" || !mScreen) {
        return;
    }
    QString currentThemeMode = styleSettings->get(key).toString();
    bool isDarkThemeMode = ("ukui-black" == currentThemeMode || "ukui-dark" == currentThemeMode);
    for (QMLOutput *mOutput: mScreen->outputs()) {
        if (isDarkThemeMode) {
            mOutput->setIsDarkMode(true);
            qDebug() << Q_FUNC_INFO << "set dark mode" << currentThemeMode;
        } else {
            mOutput->setIsDarkMode(false);
        }
    }
}

int Widget::getConnectedOutputNum() {
    int connectNum = 0;
    Q_FOREACH (const auto &output, mConfig->connectedOutputs()) {
        qDebug() << Q_FUNC_INFO << "output" << output->name() << output->modes().size();
        if (output->modes().size() <= 0) {
            qDebug() << Q_FUNC_INFO << "skip output" << output->name();
            continue;
        }
        ++connectNum;
    }
    return connectNum;
}

void Widget::setConfig(const KScreen::ConfigPtr &config)
{
    if (mConfig) {
        KScreen::ConfigMonitor::instance()->removeConfig(mConfig);
        for (const KScreen::OutputPtr &output : mConfig->outputs()) {
            output->disconnect(this);
        }
        mConfig->disconnect(this);
    }
    mConfig = config;
    mPrevConfig = config->clone();
    int connectNum = getConnectedOutputNum();
    qDebug() << Q_FUNC_INFO << "connectNum" << connectNum;
    if (mMultiScreenFrame) {
        qDebug() << Q_FUNC_INFO << "connetedOutputs count" << connectNum;
        mMultiScreenFrame->setVisible(connectNum > 1);
    }
    updateMultiScreen();

    KScreen::ConfigMonitor::instance()->addConfig(mConfig);
    resetPrimaryCombo();
    connect(mConfig.data(), &KScreen::Config::outputAdded,
            this, [=](const KScreen::OutputPtr &output){
        outputAdded(output, false);
    });
    connect(mConfig.data(), &KScreen::Config::outputRemoved,
            this, [=](int outputId){
        outputRemoved(outputId, false);
        updateMultiScreen();
    });

    connect(mConfig.data(), &KScreen::Config::primaryOutputChanged,
            this, &Widget::primaryOutputChanged);

    // 上面屏幕拿取配置
    qDebug() << Q_FUNC_INFO << "mIscloneMode" << mIscloneMode;
    if (mScreen) {
        if (mIscloneMode) {
            mScreen->setCloneMode(true);
            qDebug() << Q_FUNC_INFO << "mScreen setCloneMode true";
        }
        mScreen->setConfig(mConfig);
    } else {
        qWarning() << Q_FUNC_INFO << "mScreen is null, skip QML screen config";
    }

    mControlPanel->setConfig(mConfig);
    // OutputConfig（含缩放下拉）每次重建后，打开显示器开关也需重建以排在缩放之后
    initOpenMonitorButton();
    alignScaleToOpenMonitorTabOrder();
    const QByteArray id(UKUI_STYLE_ID);
    QGSettings* styleSettings  = new QGSettings(id, QByteArray(), this);
    if (mScreen) {
        for (QMLOutput *mOutput: mScreen->outputs()) {
            QString currentThemeMode = styleSettings->get(UKUI_STYLE_NAME).toString();
            if (currentThemeMode == "ukui-dark" ||
                    currentThemeMode == "ukui-black") {
                mOutput->setIsDarkMode(true);
                qDebug() << Q_FUNC_INFO << "set dark mode" << currentThemeMode;
            }
        }
    }
    connect(styleSettings, &QGSettings::changed, this, [=, styleSettings](const QString &key) {
        outputSetDarkMode(styleSettings, key);
    });
    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        m_outputIdToNameMap.insert(output->id(), output->name());
        if (false == unifySetconfig) {
            outputAdded(output, false);
        } else { //解决统一输出之后connect信号不再触发的问题
            connect(output.data(), &KScreen::Output::isConnectedChanged,
                this, &Widget::slotOutputConnectedChanged);
            connect(output.data(), &KScreen::Output::isEnabledChanged,
                this, &Widget::slotOutputEnabledChanged);
            if (mScreen) {
                for (QMLOutput *mOutput: mScreen->outputs()) {
                    // 比较指针是否同一输出；原代码误用赋值导致恒为真
                if (mOutput->outputPtr() == output) {
                        disconnect(mOutput, SIGNAL(clicked()),
                                   this, SLOT(mOutputClicked())); //避免多次连接
                        connect(mOutput, SIGNAL(clicked()),
                                this, SLOT(mOutputClicked()));
                    }
                }
            }
        }
    }
   unifySetconfig = false;
    // 择主屏幕输出
    if (mScreen) {
        QMLOutput *qmlOutput = mScreen->primaryOutput();

        if (qmlOutput) {
            mScreen->setActiveOutput(qmlOutput);
        } else if (!mScreen->outputs().isEmpty()) {
            mScreen->setActiveOutput(mScreen->outputs().at(0));
            // 择一个主屏幕，避免闪退现象
            primaryButtonEnable(true);
        }
        // 配置异步到达时补读屏名；勿抢焦（侧栏进页焦点应留在侧栏）
        bindPreviewAccessibleNames();
    }
    slotOutputEnabledChanged();

    if (mFirstLoad) {
        if (mIscloneMode) {
            slotUnifyOutputs();
        }
        setMulScreenVisiable();
        initMultScreenStatus();
        changescale();
        compareScale();
        if (mIsSCaleRes) {
            mIsSCaleRes = false;
            showZoomtips();
        }
    }
    mFirstLoad = false;

    // 护眼区放到分辨率/亮度等创建之后，Tab 靠创建序自然落到主显示控件之后
    if (!m_nightModeUiInited) {
        initNightModeUi();
        setNightComponent();
        initNightModeConnection();
        initNightStatus();
        hideComponent();
        m_nightModeUiInited = true;
    }
    // 多次 setConfig 后 OutputConfig 可能排到护眼之后，再对齐缩放→打开显示器
    alignScaleToOpenMonitorTabOrder();
}

KScreen::ConfigPtr Widget::currentConfig() const
{
    return mConfig;
}

void Widget::initNightModeUi()
{
    ui->verticalLayout->setSpacing(8);
    m_nightModeLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_nightModeLabel, "Display", "", "");
    m_nightModeLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Display/Color Temperature And Eye Care
    m_nightModeLabel->setText(tr("Color Temperature And Eye Care"));

    // 护眼模式
    //~ contents_path /Display/Eye Protection Mode
    m_eyesModeSwitchWidget = new SwitchWidget(tr("Eye Protection Mode"), this, UkccFrame::BorderRadiusStyle::Around, tr("The screen color temperature warms up to prevent eye, and can be used all day long."));
    m_eyesModeSwitchWidget->setMinimumSize(QSize(550, 0));
    m_eyesModeSwitchWidget->setMaximumSize(QSize(16777215, 16777215));
    m_eyesModeSwitchWidget->setHeightAdaptive(false);
    KDK_EXTEND_ALL_INFO_FORMAT(m_eyesModeSwitchWidget->switchButton(), "Display", "", "");

    // 默认
    m_defaultNightModeFrame = new UkccFrame();
    m_defaultNightModeFrame->setFixedHeight(60);
    QHBoxLayout* defaultNightModeLayout = new QHBoxLayout(m_defaultNightModeFrame);
    defaultNightModeLayout->setContentsMargins(16, 16, 16, 16);
    m_defaultRadioButton = new QRadioButton(tr("System Default"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_defaultRadioButton, "Display", "", "");
    defaultNightModeLayout->addWidget(m_defaultRadioButton);
    defaultNightModeLayout->addStretch();

    // 自定义色温
    m_customNightModeFrame = new UkccFrame();
    m_customNightModeFrame->setFixedHeight(60);
    QHBoxLayout* customNightModeLayout = new QHBoxLayout(m_customNightModeFrame);
    customNightModeLayout->setContentsMargins(16, 16, 16, 16);
    m_customRadioButton = new QRadioButton(tr("Custom Color Temperature"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_customRadioButton, "Display", "", "");
    customNightModeLayout->addWidget(m_customRadioButton);
    customNightModeLayout->addStretch();

    // 强度
    m_tempFrame = new UkccFrame();
    m_tempFrame->setFixedHeight(60);
    QHBoxLayout* tempLayout = new QHBoxLayout(m_tempFrame);
    tempLayout->setContentsMargins(40, 16, 16, 16);
    KLabel* tempLabel = new KLabel(m_tempFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(tempLabel, "Display", "", "");
    tempLabel->setFixedWidth(94);
    tempLabel->setText(tr("Color Temperature Intensity"));
    KLabel* leftTempLabel = new KLabel(m_tempFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(leftTempLabel, "Display", "", "");
    leftTempLabel->setText(tr("Warmer"));
    m_tempSlider = new KSlider(Qt::Horizontal, m_tempFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_tempSlider, "Display", "", "");
    m_tempSlider->setTracking(true);
    m_tempSlider->setNodeVisible(false);
    m_tempSlider->setSliderType(KSliderType::NodeSlider);
    m_tempSlider->setMinimumWidth(0);
    m_tempSlider->setMinimumWidth(16777215);
    m_tempSlider->setSingleStep(50);
    m_tempSlider->setPageStep(50);
    m_tempSlider->setOrientation(Qt::Orientation::Horizontal);
    m_tempSlider->setRange(1100, 6500);

    KLabel* rightTempLabel = new KLabel(m_tempFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(rightTempLabel, "Display", "", "");
    rightTempLabel->setText(tr("Colder"));
    tempLayout->addWidget(tempLabel);
    tempLayout->addWidget(leftTempLabel);
    tempLayout->addWidget(m_tempSlider);
    tempLayout->addWidget(rightTempLabel);

    // 开启时间
    m_timeModeFrame = new UkccFrame();
    m_timeModeFrame->setMinimumSize(550, 60);
    m_timeModeFrame->setMaximumSize(16777215, 60);
    QHBoxLayout* timeModeLayout = new QHBoxLayout(m_timeModeFrame);
    timeModeLayout->setContentsMargins(40, 0, 16, 0);
    KLabel* timeModeLabel = new KLabel(m_timeModeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(timeModeLabel, "Display", "", "");
    timeModeLabel->setText(tr("Open Time"));
    timeModeLabel->setFixedWidth(94);
    m_timeModeComboBox = new QComboBox(m_timeModeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_timeModeComboBox, "Display", "", "");
    timeModeLayout->addWidget(timeModeLabel);
    timeModeLayout->addWidget(m_timeModeComboBox);

    // 设置时间
    m_customTimeFrame = new UkccFrame(this, UkccFrame::BorderRadiusStyle::Around, true);
    m_customTimeFrame->setFixedHeight(60);

    m_customTimeLabel = new KLabel(m_customTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_customTimeLabel, "Display", "", "");
    m_customTimeLabel->setFixedWidth(350);
    m_customTimeLabel->setText(tr("Set Custom Time"));

    m_openTimeHCombox = new QComboBox(m_customTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_openTimeHCombox, "Display", "", "");
    m_openTimeHCombox->setFixedWidth(64);

    m_openTimeMCombox = new QComboBox(m_customTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_openTimeMCombox, "Display", "", "");
    m_openTimeMCombox->setFixedWidth(64);

    m_closeTimeHCombox = new QComboBox(m_customTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_closeTimeHCombox, "Display", "", "");
    m_closeTimeHCombox->setFixedWidth(64);

    m_closeTimeMCombox = new QComboBox(m_customTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_closeTimeMCombox, "Display", "", "");
    m_closeTimeMCombox->setFixedWidth(64);

    m_toLabel = new KLabel(m_customTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_toLabel, "Display", "", "");
    m_toLabel->setFixedWidth(20);
    m_toLabel->setText(tr("To"));

    KLabel *colonLeftLabel = new KLabel(m_customTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(colonLeftLabel, "Display", "", "");
    colonLeftLabel->setFixedWidth(4);
    colonLeftLabel->setText(":");

    KLabel *colonRightLabel = new KLabel(m_customTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(colonRightLabel, "Display", "", "");
    colonRightLabel->setFixedWidth(4);
    colonRightLabel->setText(":");

    QHBoxLayout* customTimeLayout = new QHBoxLayout(m_customTimeFrame);
    customTimeLayout->setContentsMargins(40, 0, 16, 0);
    customTimeLayout->setSpacing(8);
    customTimeLayout->addWidget(m_customTimeLabel);
    customTimeLayout->addStretch();
    customTimeLayout->addWidget(m_openTimeHCombox);
    customTimeLayout->addWidget(colonLeftLabel);
    customTimeLayout->addWidget(m_openTimeMCombox);
    customTimeLayout->addWidget(m_toLabel);
    customTimeLayout->addWidget(m_closeTimeHCombox);
    customTimeLayout->addWidget(colonRightLabel);
    customTimeLayout->addWidget(m_closeTimeMCombox);

    m_nightModeFrame = new SettingGroup(this);
    m_nightModeFrame->addWidget(m_eyesModeSwitchWidget);
    m_nightModeFrame->addWidget(m_defaultNightModeFrame);
    m_nightModeFrame->addWidget(m_customNightModeFrame);
    m_nightModeFrame->addWidget(m_tempFrame);
    m_nightModeFrame->addWidget(m_timeModeFrame);
    m_nightModeFrame->addWidget(m_customTimeFrame);

    ui->verticalLayout->addWidget(m_nightModeLabel);
    ui->verticalLayout->addWidget(m_nightModeFrame);

    m_eyesModeSwitchWidget->switchButton()->setAccessibleName(tr("Eye Protection Mode"));
    m_defaultRadioButton->setAccessibleName(tr("System Default"));
    m_customRadioButton->setAccessibleName(tr("Custom Color Temperature"));
    // 仅「系统默认」用零尺寸 label 建 LABELLED_BY→护眼模式（跨 Frame setBuddy 对 AT-SPI 无效）
    // 勿 insertWidget：零尺寸控件仍会占布局间距，导致与「自定义色温」单选错位
    KLabel *eyesModeBuddyDefault = new KLabel(m_defaultNightModeFrame);
    eyesModeBuddyDefault->setText(tr("Eye Protection Mode"));
    eyesModeBuddyDefault->setFixedSize(0, 0);
    eyesModeBuddyDefault->move(0, 0);
    eyesModeBuddyDefault->setBuddy(m_defaultRadioButton);
    tempLabel->setAccessibleName(tr("Color Temperature Intensity"));
    tempLabel->setBuddy(m_tempSlider);
    m_tempSlider->setAccessibleName(tr("Color Temperature Intensity"));
    timeModeLabel->setAccessibleName(tr("Open Time"));
    timeModeLabel->setBuddy(m_timeModeComboBox);
    m_timeModeComboBox->setAccessibleName(tr("Open Time"));
    // 设置时间 / 至 绑定到起止小时下拉；分钟不下发「开始/结束小时」类角色名
    m_customTimeLabel->setAccessibleName(tr("Set Custom Time"));
    m_customTimeLabel->setBuddy(m_openTimeHCombox);
    m_toLabel->setAccessibleName(tr("To"));
    m_toLabel->setBuddy(m_closeTimeHCombox);
    m_openTimeHCombox->setAccessibleName(QString());
    m_openTimeMCombox->setAccessibleName(QString());
    m_closeTimeHCombox->setAccessibleName(QString());
    m_closeTimeMCombox->setAccessibleName(QString());

    // 互斥由业务槽维护；关闭 autoExclusive，避免进组焦点落到当前勾选项而跳过「系统默认」
    m_defaultRadioButton->setAutoExclusive(false);
    m_customRadioButton->setAutoExclusive(false);
    m_eyesModeSwitchWidget->switchButton()->setFocusPolicy(Qt::StrongFocus);
    m_defaultRadioButton->setFocusPolicy(Qt::StrongFocus);
    m_customRadioButton->setFocusPolicy(Qt::StrongFocus);
    m_tempSlider->setFocusPolicy(Qt::StrongFocus);
    m_timeModeComboBox->setFocusPolicy(Qt::StrongFocus);
    m_openTimeHCombox->setFocusPolicy(Qt::StrongFocus);
    m_openTimeMCombox->setFocusPolicy(Qt::StrongFocus);
    m_closeTimeHCombox->setFocusPolicy(Qt::StrongFocus);
    m_closeTimeMCombox->setFocusPolicy(Qt::StrongFocus);
    // 护眼区完整 Tab：开关 → 系统默认 → 自定义 → 强度 → 开启时间 → 设置时间各下拉
    QWidget::setTabOrder(m_eyesModeSwitchWidget->switchButton(), m_defaultRadioButton);
    QWidget::setTabOrder(m_defaultRadioButton, m_customRadioButton);
    QWidget::setTabOrder(m_customRadioButton, m_tempSlider);
    QWidget::setTabOrder(m_tempSlider, m_timeModeComboBox);
    QWidget::setTabOrder(m_timeModeComboBox, m_openTimeHCombox);
    QWidget::setTabOrder(m_openTimeHCombox, m_openTimeMCombox);
    QWidget::setTabOrder(m_openTimeMCombox, m_closeTimeHCombox);
    QWidget::setTabOrder(m_closeTimeHCombox, m_closeTimeMCombox);
}

void Widget::loadQml()
{
    qmlRegisterType<QMLOutput>("org.kde.kscreen", 1, 0, "QMLOutput");
    qmlRegisterType<QMLScreen>("org.kde.kscreen", 1, 0, "QMLScreen");

    qmlRegisterType<KScreen::Output>("org.kde.kscreen", 1, 0, "KScreenOutput");
    qmlRegisterType<KScreen::Edid>("org.kde.kscreen", 1, 0, "KScreenEdid");
    qmlRegisterType<KScreen::Mode>("org.kde.kscreen", 1, 0, "KScreenMode");

    ui->quickWidget->setSource(QUrl("qrc:/qml/main.qml"));

    QQuickItem *rootObject = ui->quickWidget->rootObject();
    if (!rootObject) {
        qWarning() << Q_FUNC_INFO << "main.qml rootObject is null, status"
                    << ui->quickWidget->status()
                    << ui->quickWidget->errors();
        mScreen = nullptr;
        return;
    }
    // 先按名取 QObject 再转换，避免类型过滤漏掉 QQmlElement<QMLScreen>
    QObject *outputViewObj = rootObject->findChild<QObject *>(QStringLiteral("outputView"));
    mScreen = qobject_cast<QMLScreen *>(outputViewObj);
    if (!mScreen) {
        mScreen = rootObject->findChild<QMLScreen *>();
    }
    ui->mainframe->layout()->setAlignment(Qt::AlignmentFlag::AlignCenter);

    if (!mScreen) {
        qWarning() << Q_FUNC_INFO << "QMLScreen outputView not found, status"
                    << ui->quickWidget->status()
                    << "errors" << ui->quickWidget->errors()
                    << "namedObj" << outputViewObj
                    << (outputViewObj ? outputViewObj->metaObject()->className() : "null");
        return;
    }
    connect(mScreen, &QMLScreen::released, this, [=]() {
        delayApply();
    });
    connect(mScreen, &QMLScreen::focusedOutputChanged,
            this, &Widget::slotFocusedOutputChanged);
}

void Widget::resetPrimaryCombo()
{
    // 清空到重新 addItem/setCurrentIndex 全程挡信号，避免 setConfig 早期 mScreen
    // 尚未 setConfig 时 currentIndexChanged→setActiveOutputByCombox 重入/空指针
    const bool blocked = monitorComboBox->blockSignals(true);
    monitorComboBox->clear();

    if (!mConfig) {
        monitorComboBox->blockSignals(blocked);
        return;
    }

    for (auto &output: mConfig->outputs()) {
        addOutputToPrimaryCombo(output);
    }
    monitorComboBox->blockSignals(blocked);
}

void Widget::addOutputToPrimaryCombo(const KScreen::OutputPtr &output)
{
    // 注释后让他显示全部屏幕下拉框
    if (!output->isConnected()) {
        return;
    }

    monitorComboBox->addItem(UtilsOfDisplay::outputName(output), output->id());
    if (output->isPrimary() && !mIsWayland) {
        Q_ASSERT(mConfig);
        int lastIndex = monitorComboBox->count() - 1;
        monitorComboBox->setCurrentIndex(lastIndex);
    }
}

// 这里从屏幕点击来读取输出
void Widget::slotFocusedOutputChanged(QMLOutput *output)
{
    mControlPanel->activateOutput(output->outputPtr());

    // 读取屏幕点击选择下拉框
    Q_ASSERT(mConfig);
    int index = output->outputPtr().isNull() ? 0 : monitorComboBox->findData(output->outputPtr()->id());
    if (index == -1 || index == monitorComboBox->currentIndex()) {
        return;
    }
    monitorComboBox->setCurrentIndex(index);
}

void Widget::slotFocusedOutputChangedNoParam()
{
    mControlPanel->activateOutput(res);
}

void Widget::slotOutputEnabledChanged()
{
    clearOutputIdentifiers();
    // 点击禁用屏幕输出后的改变
    resetPrimaryCombo();
    // 双屏及以下保持原提前退出；三屏及以上必须扫完全部输出，否则第 3 路亮度条不刷新。
    // 多屏开关可见性已由 setMulScreenVisiable() 负责，不再依赖此处计数做 UI。
    const bool needScanAll = mConfig->connectedOutputs().count() >= 3;
    int enabledOutputsCount = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, mConfig->outputs()) {
        for (int i = 0; i < BrightnessFrameV.size(); ++i) {
            if (BrightnessFrameV[i]->getOutputName() == UtilsOfDisplay::outputName(output)) {
                BrightnessFrameV[i]->setOutputEnable(output->isEnabled());
                break;
            }
        }
        if (output->isEnabled()) {
            ++enabledOutputsCount;
            for (int i = 0; i < BrightnessFrameV.size(); ++i) {
                if (BrightnessFrameV[i]->getOutputName() == UtilsOfDisplay::outputName(output) && !BrightnessFrameV[i]->getSliderEnable()) {
                    BrightnessFrameV[i]->runConnectThread(true);
                }
            }
        }
        if (!needScanAll && enabledOutputsCount > 1) {
            break;
        }
    }
    if (needScanAll && !BrightnessFrameV.isEmpty()) {
        showBrightnessFrame();
    }
}

void Widget::slotOutputConnectedChanged()
{
    clearOutputIdentifiers();
    const KScreen::OutputPtr output(qobject_cast<KScreen::Output *>(sender()), [](void *){});
    if (!output) {
        return;
    }

    if (output->isConnected()) {
        outputAdded(output, true);
    } else {
        outputRemoved(output->id(), true);
    }

    updateMultiScreen();
    resetPrimaryCombo();

    // bug#89064,bug#89174
    QTimer::singleShot(500, this, [=](){
        setMulScreenVisiable();
        resetPrimaryCombo();
    });
}

void Widget::slotUnifyOutputs()
{
    if (mConfigChanged && (changeItm == RESOLUTION || changeItm == ORIENTATION || changeItm == FREQUENCY)) { // updateScreenConfig会更新旧配置导致恢复分辨率失败
        qDebug() << Q_FUNC_INFO << "mConfigChanged changeItm" << changeItm << "skip slotUnifyOutputs";
        return;
    }

    bool isExtendMode = (mKdsStatus == 2);
    qDebug() << Q_FUNC_INFO << "isExtendMode" << isExtendMode;
    QMLOutput *base = mScreen->primaryOutput();

    QList<int> clones;
    if (updateScreenConfig() != 0) {
        return;
    }
    if (!base) {
        for (QMLOutput *output: mScreen->outputs()) {
            if (output->output()->isConnected() && output->output()->isEnabled()) {
                base = output;
                break;
            }
        }
        if (!base) {
            // WTF?
            return;
        }
    }
    for (QMLOutput *output: mScreen->outputs()) {
        //修改镜像时usd会修改modeId
        qDebug() << "ukcc screen output" << output->outputPtr()->name() << output->output()->isConnected() << output->output()->isEnabled();
        for (KScreen::OutputPtr m_output : mPrevConfig->connectedOutputs()) {
            if (m_output->name() == output->outputPtr()->name()) {
                QString modeId = m_output->currentModeId();
                output->outputPtr()->setCurrentModeId(modeId);
                qDebug() << Q_FUNC_INFO << m_output->name() << "set currentModeId" << modeId << m_output->currentMode();
            }
        }
        if (mIscloneMode && output == base) {
            output->setIsCloneMode(true, true);
        } else {
            output->setIsCloneMode(mIscloneMode, false);
        }
    }
    // 取消统一输出
    if (!mIscloneMode) {

        unifySetconfig = true;
        qDebug() << Q_FUNC_INFO << "is not clone Mode, setConfig";
        setConfig(mPrevConfig);

        monitorComboBox->setEnabled(isExtendMode);
        openMonitorFrame->setVisible(mPrevConfig->connectedOutputs().count() > 1);
        mControlPanel->setCloneMode(false);
    } else if (mIscloneMode) {
        // Clone the current config, so that we can restore it in case user
        // breaks the cloning

        for (QMLOutput *output: mScreen->outputs()) {
            if (output != base) {
                output->output()->setRotation(base->output()->rotation());
            }

            if (!output->output()->isConnected()) {
                continue;
            }

            if (!output->output()->isEnabled()) {
                continue;
            }

            if (!base) {
                base = output;
            }

            output->setOutputX(0);
            output->setOutputY(0);
            output->output()->setPos(QPoint(0, 0));
            output->output()->setClones(QList<int>());

            if (base != output) {
                clones << output->output()->id();
                output->setCloneOf(base);
                qDebug() << Q_FUNC_INFO << "output set visible to false" << output->output()->name();
                output->setVisible(false);
            }
        }
        qDebug() << Q_FUNC_INFO << "slotUnifyOutputs base output" << base->output()->name() << "clone.size"<< clones.size() << clones;
        base->output()->setClones(clones);
        mScreen->updateOutputsPlacement();

        // 关闭开关
        qDebug() << Q_FUNC_INFO << "update multiScreen status connnectedOutputs.size" << mConfig->connectedOutputs().count() << mScreen->outputs().size();
        showBrightnessFrame(); // 针对拔插后还是镜像模式，更新亮度条是否显示
        setMulScreenVisiable();
        initMultScreenStatus(VICE);
        if (openMonitorButton) {
            openMonitorButton->setEnabled(false);
        }
        if (openMonitorFrame) {
            openMonitorFrame->setVisible(false);
        }
        monitorComboBox->setEnabled(false);
        monitorBtn->setVisible(false);
        mControlPanel->setUnifiedOutput(base->outputPtr());
        qDebug() << Q_FUNC_INFO << "after mControlPanel->setUnifiedOutput";
        mControlPanel->setCloneMode(true);
    }
}

// FIXME: Copy-pasted from KDED's Serializer::findOutput()
KScreen::OutputPtr Widget::findOutput(const KScreen::ConfigPtr &config, const QVariantMap &info)
{
    KScreen::OutputList outputs = config->outputs();
    Q_FOREACH (const KScreen::OutputPtr &output, outputs) {
        if (!output->isConnected()) {
            continue;
        }

        const QString outputId
            = (output->edid()
               && output->edid()->isValid()) ? output->edid()->hash() : output->name();
        if (outputId != info[QStringLiteral("id")].toString()) {
            continue;
        }

        QVariantMap posInfo = info[QStringLiteral("pos")].toMap();
        QPoint point(posInfo[QStringLiteral("x")].toInt(), posInfo[QStringLiteral("y")].toInt());
        output->setPos(point);
        output->setPrimary(info[QStringLiteral("primary")].toBool());
        output->setEnabled(info[QStringLiteral("enabled")].toBool());
        output->setRotation(static_cast<KScreen::Output::Rotation>(info[QStringLiteral("rotation")].
                                                                   toInt()));

        QVariantMap modeInfo = info[QStringLiteral("mode")].toMap();
        QVariantMap modeSize = modeInfo[QStringLiteral("size")].toMap();
        QSize size(modeSize[QStringLiteral("width")].toInt(),
                   modeSize[QStringLiteral("height")].toInt());

        const KScreen::ModeList modes = output->modes();
        Q_FOREACH (const KScreen::ModePtr &mode, modes) {
            if (mode->size() != size) {
                continue;
            }
            if (QString::number(mode->refreshRate())
                != modeInfo[QStringLiteral("refresh")].toString()) {
                continue;
            }

            output->setCurrentModeId(mode->id());
            break;
        }
        return output;
    }

    return KScreen::OutputPtr();
}

void Widget::initComponent()
{
    QHBoxLayout *multiScreenlay = new QHBoxLayout();
    mMultiScreenLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mMultiScreenLabel, "Display", "", "");
    mMultiScreenLabel->setText(tr("Multi-screen"));
    mMultiScreenLabel->setFixedSize(108, 30);

    mMultiScreenCombox = new QComboBox(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mMultiScreenCombox, "Display", "", "");
    mMultiScreenCombox->addItem(tr("First Screen"),  "firstScreenMode");
    mMultiScreenCombox->addItem(tr("Vice Screen"),   "secondScreenMode");
    mMultiScreenCombox->addItem(tr("Extend Screen"), "extendScreenMode");
    mMultiScreenCombox->addItem(tr("Clone Screen"),  "cloneScreenMode");
#ifndef BUILD_NO_SPLICE_SCREEN
    mMultiScreenCombox->addItem(tr("Splice Screen"));
#endif
    multiScreenlay->setContentsMargins(16, 0, 16, 0);
    multiScreenlay->addWidget(mMultiScreenLabel);
    multiScreenlay->setSpacing(16);
    multiScreenlay->addWidget(mMultiScreenCombox);

    mMultiScreenFrame->setLayout(multiScreenlay);
    mMultiScreenFrame->setVisible(false);

    if (m_StatusDbus->isValid()) {
        QDBusReply<bool> is_tabletmode = m_StatusDbus->call("get_current_tabletmode");
        if (is_tabletmode) {
            mMultiScreenCombox->setEnabled(false);
        }
    }

    mMultiScreenLabel->setAccessibleName(tr("Multi-screen"));
    mMultiScreenLabel->setBuddy(mMultiScreenCombox);
    mMultiScreenCombox->setAccessibleName(tr("Multi-screen"));
}

void Widget::initDbusComponent()
{
    dbusEdid = new QDBusInterface("org.kde.KScreen",
            "/backend",
            "org.kde.kscreen.Backend",
            QDBusConnection::sessionBus());

    mUsdDbus = new QDBusInterface("org.ukui.SettingsDaemon",
                                  "/org/ukui/SettingsDaemon/xrandr",
                                  "org.ukui.SettingsDaemon.xrandr",
                                  QDBusConnection::sessionBus(), this);


    if (mUsdDbus->isValid()) {
        QDBusReply<int> reply = mUsdDbus->call("getScreenMode", "ukui-control-center");
        mKdsStatus = reply.value();
        (reply == USD_CLONE_MODE) ? mIscloneMode = true : mIscloneMode = false;
    }

    m_StatusDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                      "/",
                                      "com.kylin.statusmanager.interface",
                                      QDBusConnection::sessionBus(), this);
}

void Widget::setHideModuleInfo()
{
    mCPU = getCpuInfo();
    //fix bug#78013
    ui->quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
    ui->quickWidget->setClearColor(Qt::transparent);
}

void Widget::writeScale(double scale)
{
    if (scale != scaleGSettings->get(SCALE_KEY).toDouble()) {
        mIsScaleChanged = true;
    }

    if (mIsScaleChanged) {
        QStringList keys = scaleGSettings->keys();
        if (keys.contains("scalingFactor")) {
            scaleGSettings->set(SCALE_KEY, scale);
        }
        if (!mIsChange) {  //主动切换缩放率，直接提示注销
            showZoomtips();
        } else {
            mIsChange = false;
        }
        mIsScaleChanged = false;

    }
}

void Widget::colorSettingChangedSlot(const QString &key) {
    qDebug() << Q_FUNC_INFO << "colorSettingChangedSlot" << key;
    if (key == "nightLightTemperature") {
        int value = m_colorSettings->get(NIGHT_TEMPERATURE_KEY).toInt();
        m_tempSlider->setValue(value);
        qDebug() << Q_FUNC_INFO << "init night-light-temperature" << value;
    } else if (key == "nightLightScheduleAutomatic" || key == "nightLightEnabled" || key == "nightLightAllday") {
        setNightModeSetting();
    } else if (key == "eyeCare") {
        m_isEyeCareMode = m_colorSettings->get(EYES_MODE).toBool();
        qDebug() << Q_FUNC_INFO << "colorSettingChangedSlot" << key << "value" << m_isEyeCareMode;
        eyesModeFrameSlot(m_isEyeCareMode || m_isNightMode);
    } else if (key == "mainSwitch") {
        bool status = m_colorSettings->get(MAIN_SWITCH).toBool();
        qDebug() << Q_FUNC_INFO << "main-switch" << status;
        if (!status) {
            m_eyesModeSwitchWidget->switchButton()->setChecked(false);
            qDebug() << Q_FUNC_INFO << "m_eyesModeSwitchButton set state close";
        } else {
            m_eyesModeSwitchWidget->switchButton()->setChecked(true);
            qDebug() << Q_FUNC_INFO << "m_eyesModeSwitchButton set state true";
        }
    }
}

void Widget::openTimeHComboxIdxChangeSlot() {
    if (m_colorSettings) {
        m_colorSettings->set(NIGHT_FROM_KEY,QString::number(hour_minute_to_value((m_openTimeHCombox->currentText()).toInt(),(m_openTimeMCombox->currentText()).toInt()),'f', 2).toDouble());
    } else {
        applyNightModeSlot();
    }
    UkccCommon::buriedSettings(QString("display"), QString("m_openTimeHCombox"), QString("select"), m_openTimeHCombox->currentText());
}

void Widget::openTimeMComboxIdxChangeSlot() {
    if (m_colorSettings) {
        m_colorSettings->set(NIGHT_FROM_KEY,QString::number(hour_minute_to_value((m_openTimeHCombox->currentText()).toInt(),(m_openTimeMCombox->currentText()).toInt()),'f', 2).toDouble());
    } else {
        applyNightModeSlot();
    }
    UkccCommon::buriedSettings(QString("display"), QString("m_openTimeMCombox"), QString("select"), m_openTimeMCombox->currentText());
}

void Widget::closeTimeHComboxIdxChangeSlot() {
    if (m_colorSettings) {
         m_colorSettings->set(NIGHT_TO_KEY, QString::number(hour_minute_to_value((m_closeTimeHCombox->currentText()).toInt(),(m_closeTimeMCombox->currentText()).toInt()),'f', 2).toDouble());
    } else {
        applyNightModeSlot();
    }
    UkccCommon::buriedSettings(QString("display"), QString("m_closeTimeHCombox"), QString("select"), m_closeTimeHCombox->currentText());
}

void Widget::closeTimeMComboxIdxChangeSlot() {
    if (m_colorSettings) {
        m_colorSettings->set(NIGHT_TO_KEY,QString::number(hour_minute_to_value((m_closeTimeHCombox->currentText()).toInt(),(m_closeTimeMCombox->currentText()).toInt()),'f', 2).toDouble());
    } else {
        applyNightModeSlot();
    }
    UkccCommon::buriedSettings(QString("display"), QString("m_closeTimeMCombox"), QString("select"), m_closeTimeMCombox->currentText());
}

void Widget::timeModeComboBoxIdxChangeSlot() {
    int currentIndex = m_timeModeComboBox->currentIndex();
    switch (currentIndex) {
    case 0:
        m_customTimeFrame->hide();
        if (m_colorSettings) {
            m_colorSettings->set(AllDAY_KEY,true);
            m_colorSettings->set(AUTO_KEY,false);
        }
        break;
    case 1:
        m_customTimeFrame->hide();
        if (m_colorSettings) {
            m_colorSettings->set(AUTO_KEY,true);
            m_colorSettings->set(AllDAY_KEY,false);
        }
        break;
    case 2:
        m_customTimeFrame->setVisible(true);
        if (m_colorSettings) {
            m_colorSettings->set(AUTO_KEY,false);
            m_colorSettings->set(AllDAY_KEY,false);
        }
        break;
    }
    UkccCommon::buriedSettings(QString("display"), QString("m_timeModeComboBox"), QString("select"), QString::number(m_timeModeComboBox->currentIndex()));
}

void Widget::defaultRadioButtonClickSlot() {
    m_defaultNightModeFrame->setVisible(true);
    m_customNightModeFrame->setVisible(true);
    m_defaultRadioButton->setChecked(true);
    m_customRadioButton->setChecked(false);

    m_tempFrame->setVisible(false);
    m_timeModeFrame->setVisible(false);
    m_customTimeFrame->setVisible(false);
    if (m_colorSettings) {
        m_colorSettings->set(EYES_MODE, true);
    }
    UkccCommon::buriedSettings(QString("display"), "set default eyes mode", QString("clicked"), "");
}

void Widget::customRadioButtonClickSlot() {
    m_eyesModeSwitchWidget->switchButton()->blockSignals(true);
    m_eyesModeSwitchWidget->switchButton()->setChecked(true);
    m_eyesModeSwitchWidget->switchButton()->blockSignals(false);
    m_customRadioButton->setChecked(true);
    m_defaultRadioButton->setChecked(false);
    m_tempFrame->setVisible(true);
    m_timeModeFrame->setVisible(true);
    m_customTimeFrame->setVisible(false);
    m_timeModeComboBox->setCurrentIndex(0);
    if (m_colorSettings) {
        m_colorSettings->set(NIGHT_ENABLE_KEY, true);
    }
    UkccCommon::buriedSettings(QString("display"), "set custom eyes mode", QString("clicked"), "");
}

void Widget::initGSettings()
{
    QByteArray id(UKUI_CONTORLCENTER_PANEL_SCHEMAS);
    if (QGSettings::isSchemaInstalled(id)) {
        m_gsettings = new QGSettings(id, QByteArray(), this);
    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.control-center.panel.plugins not install";
    }

    QByteArray nightId(SETTINGS_DAEMON_COLOR_SCHEMAS);
    if(QGSettings::isSchemaInstalled(nightId)) {
        m_colorSettings = new QGSettings(nightId);

        if (m_colorSettings) {
            connect(m_colorSettings, &QGSettings::changed, [=](const QString &key){
                colorSettingChangedSlot(key);
            });
        }
    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.SettingsDaemon.plugins.color not install";
    }

    QByteArray scaleId(FONT_RENDERING_DPI);
    if (QGSettings::isSchemaInstalled(scaleId)) {
        scaleGSettings = new QGSettings(scaleId, QByteArray(), this);
    }
}

void Widget::setcomBoxScale()
{
    int scale = 1;
    QComboBox *scaleCombox = findChild<QComboBox *>(QString("scaleCombox"));
    if (scaleCombox) {
        scale = ("100%" == scaleCombox->currentText() ? 1 : 2);
    }
    writeScale(scale);
}

void Widget::initEnhanceBrightUI()
{
    QDBusInterface* powerDbus = new QDBusInterface("org.ukui.powermanagement",
                                                   "/",
                                                   "org.ukui.powermanagement.interface",
                                                   QDBusConnection::systemBus(), this);
    QDBusReply<bool> isBrightnessEnhancementSupported = powerDbus->call("IsBrightnessEnhancementSupported");

    qDebug() << Q_FUNC_INFO << "isBrightnessEnhancementSupported" << isBrightnessEnhancementSupported;
    if (isBrightnessEnhancementSupported) {
        m_enhanceBrightFrame->setVisible(true);
        if (QGSettings::isSchemaInstalled(QByteArray(POWER_SCHMES))) {
            mPowerGSettings = new QGSettings(QByteArray(POWER_SCHMES), QByteArray(), this);
            if (mPowerGSettings->keys().contains("brightnessEnhancementSwitch")) {
                m_enhanceBrightOpen = mPowerGSettings->get(BRIGHTNESS_ENHANCE_SWITCH_KEY).toBool();
                m_enhanceBrightBtn->setChecked(m_enhanceBrightOpen);
                qDebug() << Q_FUNC_INFO << "initEnhanceBrightUI brightnessEnhancementSwitch" << m_enhanceBrightOpen;
                connect(m_enhanceBrightBtn, &KSwitchButton::stateChanged, this, [=](bool status) {
                    enhanceBrightBtnChanged(status);
                });
            }

            connect(mPowerGSettings, &QGSettings::changed, this, [=](const QString &key) {
                powerGsettingChangedSlot(key);
            });
        }
    } else {
        qDebug() << Q_FUNC_INFO << "isBrightnessEnhancementSupported" << isBrightnessEnhancementSupported << "set hide m_enhanceBrightFrame";
        m_enhanceBrightFrame->setVisible(false);
    }
}

void Widget::enhanceBrightBtnChanged(bool status) {
    m_enhanceBrightOpen = status;
    qDebug() << Q_FUNC_INFO << "m_enhanceBrightBtn stateChanged" << status << m_enhanceBrightOpen;
    mPowerGSettings->set(BRIGHTNESS_ENHANCE_SWITCH_KEY, status);
    for (int index = 0; index < BrightnessFrameV.size(); ++index) {
        if (!BrightnessFrameV[index]) {
            continue;
        }
        qDebug() << Q_FUNC_INFO << "enhanceBrightBtnChanged setSliderRange" << m_enhanceBrightOpen << BrightnessFrameV[index]->getOutputName();
        BrightnessFrameV[index]->setSliderRange(m_enhanceBrightOpen);
    }
    UkccCommon::buriedSettings(QString("display"), QString("m_enhanceBrightBtn"), QString("clicked"), UkccCommon::boolToString(status));
}

void Widget::powerGsettingChangedSlot(const QString &key) {
    if (!key.compare("brightnessEnhancementSwitch")) {
        m_enhanceBrightBtn->blockSignals(true);
        bool status = mPowerGSettings->get(key).toBool();
        qDebug() << Q_FUNC_INFO << "brightnessEnhancementSwitch changed" << status;
        m_enhanceBrightBtn->setChecked(status);
        m_enhanceBrightOpen = status;
        m_enhanceBrightBtn->blockSignals(false);
        for (int index = 0; index < BrightnessFrameV.size(); ++index) {
            if (!BrightnessFrameV[index]) {
                continue;
            }
            qDebug() << Q_FUNC_INFO << "enhanceBrightBtnChanged setSliderRange" << m_enhanceBrightOpen << BrightnessFrameV[index]->getOutputName();
            BrightnessFrameV[index]->setSliderRange(m_enhanceBrightOpen);
        }
    }
}

void Widget::initAutoBrihgtUI()
{

    QDBusInterface *mUsdGlobalDbus = new QDBusInterface("org.ukui.SettingsDaemon",
                                                "/GlobalSignal",
                                                "org.ukui.SettingsDaemon.GlobalSignal",
                                                QDBusConnection::sessionBus(), this);;
    QDBusReply<bool> isSupportsAutoBright = mUsdGlobalDbus->call("isPresenceLightSensor");

    if (isSupportsAutoBright) {
        if (QGSettings::isSchemaInstalled(QByteArray(USD_BRIGHT_SCHEMAS))) {
            mAutoBrightSettings = new QGSettings(QByteArray(USD_BRIGHT_SCHEMAS), QByteArray(), this);
            if (mAutoBrightSettings->keys().contains("autoBrightness")) {
                mAutoBrightFrame->setChecked(mAutoBrightSettings->get(BRIGHT_KEY).toBool());
                connect(mAutoBrightFrame->switchButton(), &KSwitchButton::stateChanged, this, [=](bool status) {
                    mAutoBrightSettings->set(BRIGHT_KEY, status);
                    UkccCommon::buriedSettings(QString("display"), QString("mAutoBrightBtn"), QString("clicked"), UkccCommon::boolToString(status));
                });
            }

            connect(mAutoBrightSettings, &QGSettings::changed, this, [=](const QString &key) {
                if (!key.compare("autoBrightness")) {
                    mAutoBrightFrame->blockSignals(true);
                    mAutoBrightFrame->setChecked(mAutoBrightSettings->get(key).toBool());
                    mAutoBrightFrame->blockSignals(false);
                }
            });
        }
    } else {
        configGroup->removeWidget(mAutoBrightFrame);
        mAutoBrightFrame->deleteLater();
        mAutoBrightFrame = nullptr;
    }
}

bool Widget::isRestoreConfig()
{
    if (autoScaleChange) {
        autoScaleChange = false;
        return false;
    }
    int cnt = 15;
    QMessageBox msg(qApp->activeWindow());
    QPushButton *cancelBtn = msg.addButton(tr("Not Save"), QMessageBox::RejectRole);
    QPushButton *saveBtn = msg.addButton(tr("Save"), QMessageBox::AcceptRole);
    msg.setDefaultButton(saveBtn);
    cancelBtn->setProperty("useButtonPalette", true);
    msg.setParent(this);

    QString config_name;
    if (mConfigChanged) {
        switch (changeItm) {
        case RESOLUTION:
            config_name = tr("Resolution");
            break;
        case ORIENTATION:
            config_name = tr("Orientation");
            break;
        case FREQUENCY:
            config_name = tr("Frequency");
            break;
        case SCALE:
            config_name = tr("Scale");
            break;
        }
        msg.setIcon(QMessageBox::Warning);
        msg.setText(QString(tr("The screen %1 has been modified, whether to save it ?")).arg(config_name));
        msg.setInformativeText(QString(tr("The settings will be restore after 14 seconds")));
        QTimer cntDown;
        QObject::connect(&cntDown, &QTimer::timeout, [&msg, &cnt, &cntDown]()->void {
            if (--cnt < 0) {
                cntDown.stop();
                msg.hide();
                msg.close();
            } else {
                // 只刷新倒计时文案。勿再 msg.show()：会把焦点抢回默认「保存」，Tab 到「不保存」后被拽回
                QWidget *prevFocus = qApp->focusWidget();
                msg.setInformativeText(QString(tr("The settings will be restore after %1 seconds")).arg(cnt));
                if (prevFocus && msg.isAncestorOf(prevFocus)) {
                    prevFocus->setFocus(Qt::OtherFocusReason);
                }
            }
        });
        cntDown.start(1000);
        msg.exec();
    } else {
        if (mIsSCaleRes) {
            mIsSCaleRes = false;
            showZoomtips();
        }
        return false;
    }

    if (msg.clickedButton() == saveBtn) {
        //若点击保存后，因分辨率导致缩放发生了变化，则提示注销
        if (mIsSCaleRes) {
            mIsSCaleRes = false;
            showZoomtips();
        }
        return false;
    } else {
        if (mIsSCaleRes) {
            QStringList keys = scaleGSettings->keys();
            if (keys.contains("scalingFactor")) {
                scaleGSettings->set(SCALE_KEY,scaleres);
                compareScale();
            }
            mIsSCaleRes = false;
        }
        UkccCommon::buriedSettings(QString("display"), QString("settings not saved"), QString("settings"), config_name);
        return true;
    }
}

QString Widget::getCpuInfo()
{
    return UkccCommon::getCpuInfo();
}

bool Widget::isCloneMode()
{
    KScreen::OutputPtr output = mConfig->primaryOutput();
    if (!output) {
        return false;
    }
    if (mConfig->connectedOutputs().count() >= 2) {
        foreach (KScreen::OutputPtr secOutput, mConfig->connectedOutputs()) {
            //不能用size为(-1,-1)进行判断，刚插拔时分辨率可能会是(-1,-1)
            if (secOutput->pos() != output->pos() || !secOutput->isEnabled()) {
                return false;
            }
        }
    } else {
        return false;
    }
    return true;
}

bool Widget::isBacklight()
{
    QDBusInterface ukccIfc("com.control.center.qt.systemdbus",
                           "/",
                           "com.control.center.interface",
                           QDBusConnection::systemBus());

    QDBusReply<QString> m_reply;
    m_reply = ukccIfc.call("getDmidecodeType");
    if (m_reply.isValid()) {
        mMachineType = m_reply.value();
        if (m_reply.value() == "all in one") { //一体机
            return isBacklightAllInOne();
        } else if (m_reply.value() == "notebook") { //笔记本
            return true;
        } else if (m_reply.value() == "desktop") { //台式机
            return false;
        }
    }
    QDBusInterface brightnessInterface("org.freedesktop.UPower",
                                       "/org/freedesktop/UPower",
                                       "org.freedesktop.DBus.Properties",
                                       QDBusConnection::systemBus());
    if (!brightnessInterface.isValid()) {
        qDebug() << "Create UPower Interface Failed : " << QDBusConnection::systemBus().lastError();
        return false;
    }
    QDBusReply<QVariant> reply = brightnessInterface.call("Get", "org.freedesktop.UPower", "LidIsPresent");
    if (!reply.isValid()) {
        qDebug()<<"Get LidIsPresent Failed";
    }
    return reply.value().toBool();
}

bool Widget::isPanguX()
{
    QDBusInterface ukccIfc("com.control.center.qt.systemdbus",
                           "/",
                           "com.control.center.interface",
                           QDBusConnection::systemBus());

    QDBusReply<QString> reply;
    reply = ukccIfc.call("getDmiDecodeRes", "baseboard");
    qDebug() << Q_FUNC_INFO << reply.value();
    if (reply.value().contains("SP1PANGUXM")) {
        return true;
    }
    return false;
}

bool Widget::isCe72()
{
    QDBusInterface ukccIfc("com.control.center.qt.systemdbus",
                           "/",
                           "com.control.center.interface",
                           QDBusConnection::systemBus());

    QDBusReply<QString> reply;
    reply = ukccIfc.call("getDmiDecodeRes", "-t 1");
    qDebug() << Q_FUNC_INFO << reply.value();
    if (reply.value().contains("CE720Z2") && reply.value().contains("inspur")) {
        return true;
    }
    return false;
}

QString Widget::getMonitorType()
{
    QString monitor = monitorComboBox->currentText();
    QString type;
    if (monitor.contains("VGA", Qt::CaseInsensitive)) {
        type = "4";
    } else {
        type = "8";
    }
    return type;
}

bool Widget::isLaptopScreen()
{
    int index = monitorComboBox->currentIndex();
    KScreen::OutputPtr output = mConfig->output(monitorComboBox->itemData(index).toInt());
    if (output->type() == KScreen::Output::Type::Panel) {
        return true;
    }
    return false;
}

bool Widget::isVisibleBrightness()
{
    if ((mIsBattery && isLaptopScreen())
            || (mIsWayland && !mIsBattery)
            || (!mIsWayland && mIsBattery)) {
        return true;
    }
    return false;
}


int Widget::getPrimaryScreenID()
{
    QString primaryScreen = getPrimaryWaylandScreen();
    int screenId;
    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        if (!output->name().compare(primaryScreen, Qt::CaseInsensitive)) {
            screenId = output->id();
        }
    }
    return screenId;
}

void Widget::setScreenIsApply(bool isApply)
{
    mIsScreenAdd = !isApply;
}

void Widget::setMulScreenVisiable()
{
    int connectNum = getConnectedOutputNum();
    qDebug() << Q_FUNC_INFO << "connectNum" << connectNum;
    bool isMult = connectNum >= 2 ? true : false;
    mMultiScreenFrame->setVisible(isMult);
    initMultScreenStatus();
}

void Widget::initMultScreenStatus(int status)
{
    mMultiScreenCombox->blockSignals(true);
    QDBusReply<int> reply = mUsdDbus->call("getScreenMode", "ukui-control-center");
    int mode = reply.value();
    qDebug() << Q_FUNC_INFO << "getScreenMode" << mode;
    if (mMultiScreenCombox->count() <= 2) {
        mode = ((mode == 1) ? 3 : 0);
    }
    switch (mode) {
    case 0:
        mMultiScreenCombox->setCurrentIndex(FIRST);
        break;
    case 1:
        mMultiScreenCombox->setCurrentIndex(CLONE);
        break;
    case 2:
        mMultiScreenCombox->setCurrentIndex(EXTEND);
        break;
    case 3:
        mMultiScreenCombox->setCurrentIndex(VICE);
        break;
    //spliceScreen
    default:
        break;
    }
    mMultiScreenCombox->blockSignals(false);
    qDebug() << Q_FUNC_INFO << "screen mode" << mode << "MultiScreenCombox count" << mMultiScreenCombox->count();
    if ((mMultiScreenCombox->count() != 2 && mode != 2) || (mMultiScreenCombox->count() == 2 && mode != 0)) { // 非扩展模式隐藏设置主屏
        m_primaryLine->setVisible(false);
        m_primaryFrame->setVisible(false);
    } else {
        m_primaryLine->setVisible(true);
        m_primaryFrame->setVisible(true);
    }
}

void Widget::updateMultiScreen()
{
    int index = 0;
    QVector<KScreen::OutputPtr> outputs;
    Q_FOREACH (const auto &output, mConfig->connectedOutputs()) {
        outputs << output;
    }

    mMultiScreenCombox->blockSignals(true);
    int connectNum = getConnectedOutputNum();
    qDebug() << Q_FUNC_INFO << "connectNum" << connectNum;
    if (connectNum >= 3) {
        if (mMultiScreenCombox->count() >= 4) {
            mMultiScreenCombox->removeItem(0);
            mMultiScreenCombox->removeItem(0);
        }
    } else {
        if (mMultiScreenCombox->count() <= 2) {
            mMultiScreenCombox->insertItem(0, tr("Vice Screen"),  "secondScreenMode");
            mMultiScreenCombox->insertItem(0, tr("First Screen"), "firstScreenMode");
        }
    }

    for (const KScreen::OutputPtr output : outputs) {
        if (index < 2 && mMultiScreenCombox->count() >= 3) {
            mMultiScreenCombox->setItemText(index++, UtilsOfDisplay::outputName(output));
            qDebug() << Q_FUNC_INFO << index << UtilsOfDisplay::outputName(output) << outputs.size();
        }
    }

    mMultiScreenCombox->blockSignals(false);
}

int Widget::updateScreenConfig()
{
    if (mConfig->connectedOutputs().size() < 1) {
        return -1;
    }
    auto *preOp = new KScreen::GetConfigOperation();
    if (!preOp) {
        qDebug() << Q_FUNC_INFO << "preOp is null";
        return -1;
    }
    preOp->exec();
    if (!preOp->config()) {
        qDebug() << Q_FUNC_INFO << "KScreen::ConfigPtr is null";
        return -1;
    }
    mPrevConfig = preOp->config()->clone();  //重新获取屏幕当前状态，通过mconfig未必能获取到正确的状态
    qDebug() << Q_FUNC_INFO << "updateScreenConfig connectedOutput count" << mPrevConfig->connectedOutputs().size();
    preOp->deleteLater();
    return 0;
}

void Widget::showZoomtips()
{
    int ret;
    QMessageBox msg(this->topLevelWidget());
    msg.setIcon(QMessageBox::Warning);
    msg.setText(tr("The zoom has been modified, it will take effect after you log off"));
    QPushButton* logoutButton = msg.addButton(tr("Log out now"), QMessageBox::AcceptRole);
    QPushButton* noLogoutButton = msg.addButton(tr("Later"), QMessageBox::RejectRole);
    noLogoutButton->setProperty("useButtonPalette", true);
    msg.setDefaultButton(logoutButton);
    ret = msg.exec();

    switch (ret) {
    case QMessageBox::AcceptRole:
        system("ukui-session-tools --logout");
        break;
    case QMessageBox::RejectRole:
        compareScale();
        break;
    }
}

void Widget::compareScale()
{
    for (int index = 0;index < scaleComboBox->count(); index++) {
        if (scaleComboBox->itemText(index).contains(tr("(Effective after logout)"))) {
            QString str = scaleComboBox->itemText(index);
            scaleComboBox->setItemText(index, str.split(" ").at(0));
            break;
        }
    }
    double scale = scaleGSettings->get(SCALE_KEY).toDouble();
    if (scale != qApp->devicePixelRatio())
        scaleComboBox->setItemText(scaleComboBox->currentIndex(),
                                    QString("%1%2").arg(QString::number(scale * 100) + "% ")
                                    .arg(tr("(Effective after logout)")));
}

void Widget::outputEnableChanged()
{
    changescale();
    if (mIsSCaleRes) {
        mIsSCaleRes = false;
        showZoomtips();
    }
}

void Widget::clearOutputIdentifiers()
{
    mOutputTimer->stop();
    qDeleteAll(mOutputIdentifiers);
    mOutputIdentifiers.clear();
}

void Widget::addBrightnessFrame(QString name, bool openFlag, QString edidHash, const KScreen::OutputPtr &output)
{
#ifdef BUILD_FOR_NO_BRIGHTNESS
    if (!mIsBattery) {
        return;
    }
#endif

    if (m_isPanguX || m_isCe72 || (!m_isSupportGM && !mIsBattery)) {
        qDebug() << Q_FUNC_INFO << m_isPanguX << !m_isSupportGM;
        return;
    }

    qDebug() << Q_FUNC_INFO << mIsBattery << name << firstAddOutputName << openFlag << edidHash << m_isSetGM;
    // bug#189369
    QString productName = UkccCommon::getProductName();
    if ((mIsBattery && name != firstAddOutputName) ||
            (productName != "MT401" && productName.startsWith("MT") && !mIsBattery)) { //笔记本非内置
        return;
    }

    if (mIsBattery) { //移除之前的亮度条，适用于kscreen返回当前为笔记本屏幕，但之前已经把第一个屏幕当做笔记本屏幕的情况
        for (int i = 0; i < BrightnessFrameV.size(); i = 0) {
            BrightnessFrameV[BrightnessFrameV.size() - 1]->deleteLater();
            BrightnessFrameV[BrightnessFrameV.size() - 1] = nullptr;
            BrightnessFrameV.pop_back();
        }
    }

    for (int i = 0; i < BrightnessFrameV.size(); ++i) {  //已经有了
        if (name == BrightnessFrameV[i]->getOutputName()) {
            const bool connected = output->isConnected();
            const bool enable = openFlag || connected;
            if (edidHash != BrightnessFrameV[i]->getEdidHash()) {//更换了同一接口的显示器
                BrightnessFrameV[i]->updateEdidHash(edidHash);
                BrightnessFrameV[i]->setSliderEnable(false);
                BrightnessFrameV[i]->runConnectThread(enable);
            } else if (connected) {
                BrightnessFrameV[i]->runConnectThread(true);
            }
            BrightnessFrameV[i]->setOutputEnable(enable);
            return;
        }
    }
    BrightnessFrame *frame = nullptr;
    if (mIsBattery && name == firstAddOutputName) {
        frame = new BrightnessFrame(name, true, "", output);
    } else if(!mIsBattery) {
        frame = new BrightnessFrame(name, false, edidHash, output);
        if (edidI2CbusMap.isEmpty()) {
            for (QMap<QString, QString>::const_iterator it = I2CbusMap.constBegin(); it != I2CbusMap.constEnd(); ++it) {
                if (name.contains(it.key(), Qt::CaseInsensitive)) {
                    frame->setI2cbus(it.value());
                    break;
                }
            }
        } else {
            for (QMap<QString, QString>::const_iterator it = edidI2CbusMap.constBegin(); it != edidI2CbusMap.constEnd(); ++it) {
                if (edidHash == it.key()) {
                    frame->setI2cbus(it.value());
                    break;
                }
            }
        }
    }
    frame->setBrightnessByGamma(true); // openkylin默认使用gamma进行亮度调节，笔记本使用电源gsetting进行调节
    frame->setSliderRange(m_enhanceBrightOpen);
    qDebug() << Q_FUNC_INFO << "set slider range" << m_enhanceBrightOpen << output->name() << m_isSetGM;
    if (frame != nullptr) {
        connect(frame, &BrightnessFrame::sliderEnableChanged, this, [=](){
           showBrightnessFrame();
        });
        BrightnessFrameV.push_back(frame);
        mbrightnesslayout->addWidget(frame);
        frame->runConnectThread(openFlag);
        configGroup->updateShape();
    }
}

void Widget::outputAdded(const KScreen::OutputPtr &output, bool connectChanged)
{
    m_outputIdToNameMap.insert(output->id(), output->name());
    getAllI2Cbus();
    if (firstAddOutputName == "" && output->isConnected()) {
        firstAddOutputName = UtilsOfDisplay::outputName(output);
    }

    if (output->type() == 7 && output->isConnected()) { //kscreen返回为笔记本屏幕,此时不再把第一个屏幕作为笔记本屏幕
        firstAddOutputName = UtilsOfDisplay::outputName(output);
    }

    //bug:124276,特殊判断为笔记本屏幕
    if (output->type() == 14 && UkccCommon::getCpuInfo() == "Phytium,D2000/8 ULP8C" && UtilsOfDisplay::outputName(output) == "DP-2" && output->isConnected()) {
        bool flag = true;
        for (const KScreen::ModePtr &mode : output->modes()) {
            //该机器内置屏只有1920*1080分辨率
            if (mode->size() != QSize(1920,1080)) {
                flag = false;
                break;
            }
        }
        if (flag) {
            firstAddOutputName = UtilsOfDisplay::outputName(output);
        }
    }

    // bug#170141
    if (output->type() == 14 && UkccCommon::getCpuInfo() == "ZHAOXIN KaiXian KX-6640MA@2.2+GHz"&& UtilsOfDisplay::outputName(output) == "DisplayPort-0" && mMachineType == "all in one") {
        firstAddOutputName = UtilsOfDisplay::outputName(output);
    }

    if (output->isConnected()) {
        QDBusReply<QByteArray> replyEdid = dbusEdid->call("getEdid",output->id());
        const quint8 *edidData = reinterpret_cast<const quint8 *>(replyEdid.value().constData());
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.reset();
        hash.addData(reinterpret_cast<const char *>(edidData), 128);
        QString edidHash = QString::fromLatin1(hash.result().toHex());
        QString name = UtilsOfDisplay::outputName(output);
        qInfo() << "output:" << name << "edidHash:" << edidHash;
        addBrightnessFrame(name, output->isEnabled(), edidHash, output);
        connectedOutputNum++;
    }

    if (!connectChanged) {
        connect(output.data(), &KScreen::Output::isConnectedChanged,
                this, &Widget::slotOutputConnectedChanged);
        connect(output.data(), &KScreen::Output::isEnabledChanged,
                this, &Widget::slotOutputEnabledChanged);
        if (mScreen) {
            for (QMLOutput *mOutput: mScreen->outputs()) {
                if (!mOutput || mOutput->outputPtr().isNull()) {
                    qDebug() << Q_FUNC_INFO << "output is null";
                    continue;
                }
                // 比较指针是否同一输出；原代码误用赋值导致恒为真
                if (mOutput->outputPtr() == output) {
                    disconnect(mOutput, SIGNAL(clicked()),
                               this, SLOT(mOutputClicked())); //避免多次连接
                    connect(mOutput, SIGNAL(clicked()),
                            this, SLOT(mOutputClicked()));
                }
            }
        }
    }

    addOutputToPrimaryCombo(output);

    if (!mFirstLoad) {
        QTimer::singleShot(2000, this, [=] {
            mainScreenButtonSelect(monitorComboBox->currentIndex());
            setMulScreenVisiable();
            qDebug() << Q_FUNC_INFO << "after setMulScreenVisiable";
        });
    }
    updateMultiScreen();
    setMulScreenVisiable();
    showBrightnessFrame();
    // 添加显示器不能slotUnifyOutputs，否则接入3屏时会闪退，kscreen出core，未找到具体原因：libwayland-client.so报错
    if (mIscloneMode && mScreen) {
        QMLOutput *base = mScreen->primaryOutput();
        QList<int> clones;
        qDebug() << "outputAdded output name" << output->name() << "mConfig" << mConfig.data()->outputs().size() << mConfig.data()->outputs();
        Q_FOREACH (const KScreen::OutputPtr &curOutput, mConfig->outputs()) {
            if (!output || curOutput.isNull()) {
                qDebug() << Q_FUNC_INFO << "output is null, continue";
                continue;
            }
            if (!curOutput->isConnected()) {
                continue;
            }
            curOutput->setPos(QPoint(0, 0));
            curOutput->setClones(QList<int>());

            if (base->output()->name() != curOutput->name()) {
                clones << curOutput->id();
                qDebug() << Q_FUNC_INFO << "output set visible to false" << curOutput->name();
            }
        }
        qDebug() << Q_FUNC_INFO << "before setUnifiedOutput";
        if (base) {
            qDebug() << Q_FUNC_INFO << "outputAdd before setClones base output" << base->output()->name() << "clone.size"<< clones.size() << clones;
            base->output()->setClones(clones);
            mControlPanel->setUnifiedOutput(base->outputPtr());
            qDebug() << Q_FUNC_INFO << "outputAdd after mControlPanel->setUnifiedOutput";
        } else {
            qDebug() << Q_FUNC_INFO << "outputAdd base is null, skip mControlPanel->setUnifiedOutput";
        }
        mControlPanel->setCloneMode(true);
    }
}

void Widget::outputRemoved(int outputId, bool connectChanged)
{
    KScreen::OutputPtr output = mConfig->output(outputId);
    connectedOutputNum--;
    QString name = UtilsOfDisplay::outputName(output);
    for (int i = 0; i < BrightnessFrameV.size(); ++i) {
        if (name.isEmpty() && m_outputIdToNameMap.find(outputId) != m_outputIdToNameMap.end()) {
            name = m_outputIdToNameMap[outputId];
            qDebug() << Q_FUNC_INFO << "update output name" << name;
        }
        qDebug() << Q_FUNC_INFO << "BrightnessFrame name" << BrightnessFrameV[i]->getOutputName() << "remove outputid" << outputId << "remove output name" << name;
        if (BrightnessFrameV[i]->getOutputName() == name) {
            BrightnessFrameV[i]->setOutputEnable(false);
        }
    }

    if (!connectChanged) {
        if (!output.isNull()) {
            output->disconnect(this);
        }
    }

    const int index = monitorComboBox->findData(outputId);
    if (index != -1) {
        if (index == monitorComboBox->currentIndex()) {
            // We'll get the actual primary update signal eventually
            // Don't emit currentIndexChanged
            const bool blocked = monitorComboBox->blockSignals(true);
            monitorComboBox->setCurrentIndex(0);
            monitorComboBox->blockSignals(blocked);
        }

        monitorComboBox->removeItem(index);
    }

    // 检查统一输出-防止移除后没有屏幕可显示

    int outputCount = mConfig->connectedOutputs().size();
    bool modifyOutputMode = mIscloneMode && (outputCount >= 2);
    qDebug() << Q_FUNC_INFO << "modifyOutputMode" << modifyOutputMode << outputCount;
    mScreen->removeOutput(outputId);
    for (QMLOutput *qmlOutput: mScreen->outputs()) {
        if (!qmlOutput->output()->isConnected()) {
            continue;
        }
        if (!modifyOutputMode) {
            qmlOutput->setIsCloneMode(false, false);
            qDebug() << Q_FUNC_INFO << "outputRemoved set output isCloneMode to false";
        }
    }

    if (!modifyOutputMode) {
        mIscloneMode = false;
        mScreen->setCloneMode(false);
    }
    mainScreenButtonSelect(monitorComboBox->currentIndex());
    setMulScreenVisiable();
    if (mIscloneMode) {
        slotUnifyOutputs();
        qDebug() << Q_FUNC_INFO << "is cloneMode after slotUnifyOutputs";
    }
}

void Widget::primaryOutputSelected(int index)
{
    if (!mConfig) {
        return;
    }

    const KScreen::OutputPtr newPrimary = index == 0 ? KScreen::OutputPtr() : mConfig->output(monitorComboBox->itemData(index).toInt());
    if (newPrimary == mConfig->primaryOutput()) {
        return;
    }

    mConfig->setPrimaryOutput(newPrimary);
}

// 主输出
void Widget::primaryOutputChanged(const KScreen::OutputPtr &output)
{
    Q_ASSERT(mConfig);
    int index = output.isNull() ? 0 : monitorComboBox->findData(output->id());
    if (index == -1 || index == monitorComboBox->currentIndex()) {
        return;
    }
    monitorComboBox->setCurrentIndex(index);
}

void Widget::slotIdentifyButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
            this, &Widget::slotIdentifyOutputs);
}

void Widget::slotIdentifyOutputs(KScreen::ConfigOperation *op)
{
    m_firstViewResize = true;
    const KScreen::ConfigPtr config = qobject_cast<KScreen::GetConfigOperation *>(op)->config();

    mOutputTimer->stop();
    clearOutputIdentifiers();

    /* Obtain the current active configuration from KScreen */
    Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
        if (!output->isConnected() || !output->isEnabled() || !output->currentMode()) {
            continue;
        }

        const KScreen::ModePtr mode = output->currentMode();

        QQuickView *view = new QQuickView();

        view->setFlags(Qt::Tool | Qt::FramelessWindowHint);
        view->setResizeMode(QQuickView::SizeViewToRootObject);
        view->setColor(QColor(Qt::transparent)); //设置背景透明(无背景)
        view->setSource(QUrl("qrc:/qml/OutputIdentifier.qml"));
        view->installEventFilter(this);

        QQuickItem *rootObj = view->rootObject();
        if (!rootObj) {
            qWarning() << "Failed to obtain root item";
            continue;
        }

        QSize deviceSize, logicalSize;
        QPoint outputPos;
        if (output->isHorizontal()) {
            deviceSize = mode->size();
        } else {
            deviceSize = QSize(mode->size().height(), mode->size().width());
        }
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
#else
        if (config->supportedFeatures() & KScreen::Config::Feature::PerOutputScaling) {
            // no scale adjustment needed on Wayland
            logicalSize = deviceSize / output->scale();
            outputPos = output->pos();
            qDebug() << Q_FUNC_INFO << "outputname" << UtilsOfDisplay::outputName(output) << "use output scale" << output->scale();
        } else {
            logicalSize = deviceSize / devicePixelRatioF();
            outputPos = output->pos() / devicePixelRatioF();
        }
#endif

        rootObj->setProperty("outputName", UtilsOfDisplay::outputName(output));
        rootObj->setProperty("modeName", UtilsOfDisplay::sizeToString(deviceSize));

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
        view->setProperty("screenSize", QRect(output->pos(), deviceSize));
#else
        view->setProperty("screenSize", QRect(outputPos, logicalSize));
#endif
        view->resize(rootObj->size().toSize());
        mOutputIdentifiers << view;
    }

    for (QQuickView *view: mOutputIdentifiers) {
        QQuickItem *rootObj = view->rootObject();
        if (mOutputClickedName == rootObj->property("outputName").toString()) {
            view->show();
            kdk::WindowManager::setSkipTaskBar(view, true);
            kdk::WindowManager::setSkipSwitcher(view, true);
            qDebug() << Q_FUNC_INFO << "outputView show" << mOutputClickedName;
        }
    }

    mOutputTimer->start(2000);
}

void Widget::callMethod(QRect geometry, QString name)
{
    auto scale = 1;
    QDBusInterface waylandIfc("org.ukui.SettingsDaemon",
                              "/org/ukui/SettingsDaemon/wayland",
                              "org.ukui.SettingsDaemon.wayland",
                              QDBusConnection::sessionBus());

    QDBusReply<int> reply = waylandIfc.call("scale");
    if (reply.isValid()) {
        scale = reply.value();
    }

    QDBusMessage message = QDBusMessage::createMethodCall("org.ukui.SettingsDaemon",
                                                          "/org/ukui/SettingsDaemon/wayland",
                                                          "org.ukui.SettingsDaemon.wayland",
                                                          "priScreenChanged");
    message << geometry.x() / scale << geometry.y() / scale << geometry.width() / scale <<
        geometry.height() / scale << name;
    QDBusConnection::sessionBus().send(message);
}

QString Widget::getPrimaryWaylandScreen()
{
    QDBusInterface screenIfc("org.ukui.SettingsDaemon",
                             "/org/ukui/SettingsDaemon/wayland",
                             "org.ukui.SettingsDaemon.wayland",
                             QDBusConnection::sessionBus());
    QDBusReply<QString> screenReply = screenIfc.call("priScreenName");
    if (screenReply.isValid()) {
        return screenReply.value();
    }
    return QString();
}

void Widget::applyNightModeSlot()
{
    setNightMode(m_isNightMode);
}

void Widget::tempSliderChangedSlot(int tempValue)
{
    qDebug() << Q_FUNC_INFO << "tempSliderChangedSlot" << tempValue;
    m_temptSliderValue = tempValue;
    if (m_colorSettings) {
        QDBusInterface usdIfc("org.ukui.SettingsDaemon",
                              "/org/ukui/SettingsDaemon/GammaManager",
                              "org.ukui.SettingsDaemon.GammaManager",
                              QDBusConnection::sessionBus());
        usdIfc.call("setColorTemperature", "ukcc", tempValue);
    } else {
        applyNightModeSlot();
    }
}

void Widget::tempSliderReleasedSlot()
{
    qDebug() << Q_FUNC_INFO << "tempSliderReleasedSlot temptSliderValue" << m_temptSliderValue;
    if (m_colorSettings && m_temptSliderValue > 0) {
        m_colorSettings->set(NIGHT_TEMPERATURE_KEY, m_temptSliderValue);
        qDebug() << Q_FUNC_INFO << "set night-light-temperature" << m_temptSliderValue;

        QDBusInterface usdIfc("org.ukui.SettingsDaemon",
                              "/org/ukui/SettingsDaemon/GammaManager",
                              "org.ukui.SettingsDaemon.GammaManager",
                              QDBusConnection::sessionBus());
        usdIfc.call("restoreColorTemperature");
    }
    UkccCommon::buriedSettings(QString("display"), QString("m_tempSlider"), QString("settings"), QString::number(m_tempSlider->value()));
}

void Widget::setMultiScreenSlot(const QString &screenMode)
{
    qDebug() << Q_FUNC_INFO << "setMultiScreenSlot" << screenMode;
    mUsdDbus->asyncCall("setScreenMode", screenMode, "ukui-control-center");
    mUsdDbus->call("setScreenMode", screenMode, "ukui-control-center");
}

void Widget::delayApply()
{
    QTimer::singleShot(200, this, [=]() {
        // kds与插拔不触发应用操作
        if (!mIsScreenAdd) {
            save();
        }
        mIsScreenAdd = false;
    });
}

KScreen::ConfigPtr Widget::remapRestoreConfigByName(const KScreen::ConfigPtr &prevConfig) const
{
    if (!prevConfig) {
        return prevConfig;
    }

    auto *curOp = new KScreen::GetConfigOperation();
    curOp->exec();
    const KScreen::ConfigPtr liveConfig = curOp->config();
    curOp->deleteLater();
    if (!liveConfig) {
        return prevConfig;
    }

    QHash<QString, KScreen::OutputPtr> liveByName;
    for (const KScreen::OutputPtr &liveOut : liveConfig->connectedOutputs()) {
        liveByName.insert(liveOut->name(), liveOut);
    }

    QHash<int, int> idMap;
    for (const KScreen::OutputPtr &prevOut : prevConfig->connectedOutputs()) {
        const KScreen::OutputPtr liveOut = liveByName.value(prevOut->name());
        if (liveOut && liveOut->id() != prevOut->id()) {
            idMap.insert(prevOut->id(), liveOut->id());
            qDebug() << Q_FUNC_INFO << "remap output" << prevOut->name()
                     << "id" << prevOut->id() << "->" << liveOut->id();
        }
    }

    if (idMap.isEmpty()) {
        return prevConfig;
    }

    KScreen::OutputList remappedOutputs;
    for (const KScreen::OutputPtr &prevOut : prevConfig->outputs()) {
        KScreen::OutputPtr out = prevOut->clone();
        int outId = prevOut->id();
        if (idMap.contains(prevOut->id())) {
            outId = idMap.value(prevOut->id());
            out->setId(outId);
            const KScreen::OutputPtr liveOut = liveConfig->output(outId);
            if (liveOut) {
                out->setModes(liveOut->modes());
            }
        }

        QList<int> clones = out->clones();
        for (int &cloneId : clones) {
            if (idMap.contains(cloneId)) {
                cloneId = idMap.value(cloneId);
            }
        }
        out->setClones(clones);

        remappedOutputs.insert(outId, out);
    }

    KScreen::ConfigPtr restoreConfig = prevConfig->clone();
    restoreConfig->setOutputs(remappedOutputs);
    return restoreConfig;
}

void Widget::save()
{
    if (!this) {
        return;
    }


    auto *preOp = new KScreen::GetConfigOperation();
    preOp->exec();
    if (!preOp->config().isNull()) {
        mPrevConfig = preOp->config()->clone();  //重新获取屏幕当前状态
    }
    preOp->deleteLater();

    const KScreen::ConfigPtr &config = this->currentConfig();
    qDebug() << Q_FUNC_INFO << config->connectedOutputs();

    bool atLeastOneEnabledOutput = false;
    Q_FOREACH (const KScreen::OutputPtr &output, config->outputs()) {
        if (output->isEnabled()) {
            atLeastOneEnabledOutput = true;
        }
        if (!output->isConnected())
            continue;

        QMLOutput *base = mScreen->primaryOutput();
        if (!base) {
            for (QMLOutput *output: mScreen->outputs()) {
                if (output->output()->isConnected() && output->output()->isEnabled()) {
                    base = output;
                    break;
                }
            }

            if (!base) {
                // WTF?
                return;
            }
        }
    }

    if (!atLeastOneEnabledOutput) {
        QMessageBox::warning(this, tr("Warning"), tr("Please insure at least one output!"));
        if (openMonitorButton) {
            openMonitorButton->setChecked(true);
        }
        return;
    }

    if (!KScreen::Config::canBeApplied(config)) {
        QMessageBox::information(this,
                                 tr("Warning"),
                                 tr("Sorry, your configuration could not be applied.\nCommon reasons are that the overall screen size is too big, or you enabled more displays than supported by your GPU."));
        return;
    }
    mBlockChanges = true;
    /* Store the current config, apply settings */
    auto *op = new KScreen::SetConfigOperation(config);

    /* Block until the operation is completed, otherwise KCMShell will terminate
     * before we get to execute the Operation */
    op->exec();
    sync();

    // The 1000ms is a bit "random" here, it's what works on the systems I've tested, but ultimately, this is a hack
    // due to the fact that we just can't be sure when xrandr is done changing things, 1000 doesn't seem to get in the way
    QTimer::singleShot(1000, this,
                       [=]() {

        QString hash = config->connectedOutputsHash();
        writeFile(mDir % hash);
        sync();
        mBlockChanges = false;
        setMulScreenVisiable();
    });

    int enableScreenCount = 0;
    KScreen::OutputPtr enableOutput;
    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        if (output->isEnabled()) {
            enableOutput = output;
            enableScreenCount++;
        }
    }
    int delayTime = 0;
    if (changeItm == 1 || changeItm == 2) {
        delayTime = 900; //修改分辨率，为了保证弹出框居中，延时900ms
    }
    QTimer::singleShot(delayTime, this, [=]() {
        for (QMLOutput *qmlOutput : mScreen->outputs()) {
            if (!qmlOutput->allowResetSize()) {
                qmlOutput->setAllowResetSize(true);
            }
        }

        if (isRestoreConfig()) {
            mControlPanel->setRestore();
            const KScreen::ConfigPtr restoreConfig = remapRestoreConfigByName(mPrevConfig);
            auto *op = new KScreen::SetConfigOperation(restoreConfig);
            qDebug() << Q_FUNC_INFO << "before SetConfigOperation exec changeItm" << changeItm << "kscreen config" << restoreConfig;
            op->exec();
            qDebug() << Q_FUNC_INFO << "after SetConfigOperation exec changeItm" << changeItm << "kscreen config" << restoreConfig;
            sync();
        }
        mConfigChanged = false;
    });
}

QVariantMap metadata(const KScreen::OutputPtr &output)
{
    QVariantMap metadata;
    metadata[QStringLiteral("name")] = output->name();
    if (!output->edid() || !output->edid()->isValid()) {
        return metadata;
    }
    metadata[QStringLiteral("fullname")] = output->edid()->deviceId();
    return metadata;
}

QString Widget::globalFileName(const QString &hash)
{
    QString s_dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                        %QStringLiteral("/kscreen/");
    QString dir = s_dirPath  % QStringLiteral("outputs/");
    if (!QDir().mkpath(dir)) {
        return QString();
    }
    return dir % hash;
}

QVariantMap Widget::getGlobalData(KScreen::OutputPtr output)
{
    QFile file(globalFileName(output->hashMd5()));
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file" << file.fileName();
        return QVariantMap();
    }
    QJsonDocument parser;
    return parser.fromJson(file.readAll()).toVariant().toMap();
}

void Widget::writeGlobal(const KScreen::OutputPtr &output)
{
    // get old values and subsequently override
    QVariantMap info = getGlobalData(output);
    if (!writeGlobalPart(output, info, nullptr)) {
        return;
    }
    QFile file(globalFileName(output->hashMd5()));
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open global output file for writing! " << file.errorString();
        return;
    }

    file.write(QJsonDocument::fromVariant(info).toJson());
    return;
}

bool Widget::writeGlobalPart(const KScreen::OutputPtr &output, QVariantMap &info,
                             const KScreen::OutputPtr &fallback)
{
    info[QStringLiteral("id")] = output->hash();
    info[QStringLiteral("metadata")] = metadata(output);
    info[QStringLiteral("rotation")] = output->rotation();

    // Round scale to four digits
    info[QStringLiteral("scale")] = output->scale();

    QVariantMap modeInfo;
    float refreshRate = -1.;
    QSize modeSize;
    if (output->currentMode() && output->isEnabled()) {
        refreshRate = output->currentMode()->refreshRate();
        modeSize = output->currentMode()->size();
    } else if (fallback && fallback->currentMode()) {
        refreshRate = fallback->currentMode()->refreshRate();
        modeSize = fallback->currentMode()->size();
    }

    if (refreshRate < 0 || !modeSize.isValid()) {
        return false;
    }

    modeInfo[QStringLiteral("refresh")] = refreshRate;

    QVariantMap modeSizeMap;
    modeSizeMap[QStringLiteral("width")] = modeSize.width();
    modeSizeMap[QStringLiteral("height")] = modeSize.height();
    modeInfo[QStringLiteral("size")] = modeSizeMap;

    info[QStringLiteral("mode")] = modeInfo;

    return true;
}

bool Widget::writeFile(const QString &filePath)
{
    const KScreen::OutputList outputs = mConfig->outputs();
    const auto oldConfig = mPrevConfig;
    KScreen::OutputList oldOutputs;
    if (oldConfig) {
        oldOutputs = oldConfig->outputs();
    }
    QVariantList outputList;
    for (const KScreen::OutputPtr &output : outputs) {
        QVariantMap info;
        const auto oldOutputIt = std::find_if(oldOutputs.constBegin(), oldOutputs.constEnd(),
                                              [output](const KScreen::OutputPtr &out) {
            return out->hashMd5() == output->hashMd5();
        });
        const KScreen::OutputPtr oldOutput = oldOutputIt != oldOutputs.constEnd() ? *oldOutputIt
                                             : nullptr;
        if (!output->isConnected()) {
            continue;
        }

        writeGlobalPart(output, info, oldOutput);
        info[QStringLiteral("primary")] = output->isPrimary();
        info[QStringLiteral("enabled")] = output->isEnabled();

        auto setOutputConfigInfo = [&info](const KScreen::OutputPtr &out) {
                                       if (!out) {
                                           return;
                                       }

                                       QVariantMap pos;
                                       pos[QStringLiteral("x")] = out->pos().x();
                                       pos[QStringLiteral("y")] = out->pos().y();
                                       info[QStringLiteral("pos")] = pos;
                                   };
        setOutputConfigInfo(output->isEnabled() ? output : oldOutput);

        if (output->isEnabled()) {
            // try to update global output data
            writeGlobal(output);
        }
        outputList.append(info);
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open config file for writing! " << file.errorString();
        return false;
    }
    file.write(QJsonDocument::fromVariant(outputList).toJson());
    qDebug() << "Config saved on: " << file.fileName();

    return true;
}

void Widget::changedSlot()
{
    mConfigChanged = true;
}

void Widget::scaleChangedSlot(double scale)
{
    if (scaleGSettings->get(SCALE_KEY).toDouble() != scale) {
        mIsScaleChanged = true;
    } else {
        mIsScaleChanged = false;
    }
    writeScale(scale);
}

void Widget::propertiesChangedSlot(QString property, QMap<QString, QVariant> propertyMap,
                                   QStringList propertyList)
{
    Q_UNUSED(property);
    Q_UNUSED(propertyList);
    if (propertyMap.keys().contains("OnBattery")) {
        mOnBattery = propertyMap.value("OnBattery").toBool();
    }
}

// 是否禁用主屏按钮
void Widget::mainScreenButtonSelect(int index)
{
    if (!mConfig || monitorComboBox->count() <= 0) {
        return;
    }

    const KScreen::OutputPtr newPrimary = mConfig->output(monitorComboBox->itemData(index).toInt());
    if (newPrimary.isNull()) {
        return;
    }
    int connectCount = mConfig->connectedOutputs().count();

    UkccCommon::buriedSettings(QString("display"), QString("select screen/main screen"), QString("clicked"), newPrimary->name());

    if (mIsWayland) {
        if (!getPrimaryWaylandScreen().compare(newPrimary->name(), Qt::CaseInsensitive)) {
            monitorBtn->setEnabled(false);
            m_primaryCheckBox->setEnabled(false);
            m_primaryCheckBox->setChecked(true);
        } else {
            monitorBtn->setEnabled(true);
            m_primaryCheckBox->setChecked(false);
            m_primaryCheckBox->setEnabled(true);
        }
    } else {
        if (mIscloneMode || newPrimary == mConfig->primaryOutput() || mConfig->primaryOutput().isNull() || !newPrimary->isEnabled()) {
            monitorBtn->setVisible(false);
            m_primaryCheckBox->setEnabled(false);
            m_primaryCheckBox->setChecked(true);
        } else {
            monitorBtn->setVisible(true);
            m_primaryCheckBox->setEnabled(true);
            m_primaryCheckBox->setChecked(false);
        }
    }

    if (!newPrimary->isEnabled()) {
        scaleComboBox->setEnabled(false);
    } else {
        scaleComboBox->setEnabled(true);
    }

    // 设置是否勾选（开关在 setConfig 里晚于 OutputConfig 创建）
    if (openMonitorButton) {
        openMonitorButton->setEnabled(true);
        openMonitorFrame->setVisible(connectCount > 1 && !mIscloneMode);
        // 初始化时不要发射信号
        openMonitorButton->blockSignals(true);
        openMonitorButton->setChecked(newPrimary->isEnabled());
        openMonitorButton->blockSignals(false);
    } else if (openMonitorFrame) {
        openMonitorFrame->setVisible(connectCount > 1 && !mIscloneMode);
    }

    mControlPanel->activateOutput(newPrimary);

    // setConfig 里 resetPrimaryCombo 可能早于 mScreen 就绪；无屏时勿调
    if (mScreen) {
        mScreen->setActiveOutputByCombox(newPrimary->id());
    }
    // 切到另一块屏的 OutputConfig 后，缩放下拉实例变了，需重接 Tab
    alignScaleToOpenMonitorTabOrder();
}

// 设置主屏按钮
void Widget::primaryButtonEnable(bool status)
{
    Q_UNUSED(status);
    if (!mConfig) {
        return;
    }
    int index = monitorComboBox->currentIndex();
    monitorBtn->setVisible(false);
    const KScreen::OutputPtr newPrimary = mConfig->output(monitorComboBox->itemData(index).toInt());
    mConfig->setPrimaryOutput(newPrimary);
    qDebug() << Q_FUNC_INFO << "primary monitor" <<  monitorComboBox->currentText();
    for (QMLOutput *qmlOutput: mScreen->outputs()) {
        if (qmlOutput->output()->isConnected()) {
            qmlOutput->setIsClicked(false);
            qDebug() << Q_FUNC_INFO << " setclick " << qmlOutput->output()->name() << qmlOutput->isClicked();
        }
    }
}

void Widget::checkOutputScreen(bool checked)
{
    int index = monitorComboBox->currentIndex();
    KScreen::OutputPtr newPrimary = mConfig->output(monitorComboBox->itemData(index).toInt());
    if (!checked) {
        int8_t enableOutputNum = 0;
        Q_FOREACH (const KScreen::OutputPtr &output, mConfig->outputs()) {
            if (output->isEnabled()) {
                enableOutputNum++;
            }
        }

        if (enableOutputNum < 2) { //两个屏幕才允许关闭
            QMessageBox::warning(this, tr("Warning"), tr("Please insure at least one output!"));
            if (openMonitorButton) {
                openMonitorButton->blockSignals(true);
                openMonitorButton->setChecked(true);
                openMonitorButton->blockSignals(false);
            }
            return;
        }
    }

    if (mConfig->connectedOutputs().size() >= 3) {
        qDebug() << Q_FUNC_INFO << "set triple display mode" << checked;
        if (checked) {
            qDebug() << Q_FUNC_INFO << "checkOutputScreen set OpenOutput" << newPrimary->name();
        }
        qDebug() << Q_FUNC_INFO << newPrimary->name() << "is primary" << newPrimary->isPrimary();
        if (!checked && newPrimary->isPrimary()) {
            for (QMLOutput *mOutput: mScreen->outputs()) {
                if (mOutput->output()->isEnabled() && mOutput->output()->name() != newPrimary->name()) {
                    mScreen->setActiveOutput(mOutput);
                    monitorBtn->setVisible(false);
                    mConfig->setPrimaryOutput(mOutput->outputPtr());
                    qDebug() << "set output" << mOutput->output()->name() << "to primary";
                    break;
                }
            }
        }
        newPrimary->setEnabled(checked);
        if (checked) {
            for (QMLOutput *qmlOutput: mScreen->outputs()) {
                if (!qmlOutput->output()->isConnected()) {
                    continue;
                }
                if (newPrimary->name() == qmlOutput->output()->name()) {
                    qDebug() << Q_FUNC_INFO << "checkOutputScreen hit current output" << qmlOutput->output()->name();
                    mScreen->setScreenPos(qmlOutput, true);
                    qDebug() << Q_FUNC_INFO << "checkOutputScreen after setScreenPos" << qmlOutput->output()->name() << qmlOutput->position();
                    mScreen->qmlOutputMoved(qmlOutput);    // setScreenPos未调整任何顺序时，需要用qmlOutputMoved更新实际位置使其生效
                    qDebug() << Q_FUNC_INFO << "checkOutputScreen after qmlOutputMoved" << qmlOutput->output()->name() << qmlOutput->position();
                    break;
                }
            }
        }
        qDebug() << Q_FUNC_INFO << "checkOutputScreen before delayApply" << checked;
        delayApply();
    } else {
        if (checked == true) {
            if (openMonitorButton && openMonitorButton->isVisible()) {  //扩展模式
                setMultiScreenSlot("extendScreenMode");
            }
        } else {
            QString closeOutputName = UtilsOfDisplay::outputName(newPrimary);
            if (closeOutputName == mMultiScreenCombox->itemText(0)) {
                setMultiScreenSlot("secondScreenMode");
            } else if (closeOutputName == mMultiScreenCombox->itemText(1)) {
                setMultiScreenSlot("firstScreenMode");
            } else {
                qDebug()<<"(checkOutputScreen) closeOutputName = "<<closeOutputName;
            }
        }
    }
}

void Widget::usdScreenModeChangedSlot(int status)
{
    if (status == USD_CLONE_MODE && !mIscloneMode) {
        mIscloneMode = true;
    } else if (status != USD_CLONE_MODE && mIscloneMode) {
        mIscloneMode = false;
    }
    mScreen->setCloneMode(mIscloneMode);

    qDebug() << Q_FUNC_INFO << "screenMode" << status << "mIscloneMode:" << mIscloneMode;
    if (status == USD_CLONE_MODE) {
        showBrightnessFrame();
        qDebug() << Q_FUNC_INFO << "clone mode showBrightnessFrame";
    }
    int time = 1500;
    if (mIscloneMode) {
        time = 0;
        QTimer::singleShot(2500, this, [=](){ //设置镜像后，有概率会显示为扩展，原因未知，需要刷新才可以变为正常.这里处理会导致多次变化.
            this->update();
        });
    }
    qDebug() << Q_FUNC_INFO << slotTimeDelay << "status" << status << "mIscloneMode" << mIscloneMode << "mKdsStatus" << mKdsStatus;
    QTimer::singleShot(slotTimeDelay, this, [=](){   // 需要加上延时，否则mScreen获取到配置非最新
        if (mKdsStatus != status) {
            mKdsStatus = status;
            qDebug() << Q_FUNC_INFO << "before slotUnifyOutputs" << slotTimeDelay << mKdsStatus;
            slotUnifyOutputs();
            qDebug() << Q_FUNC_INFO << "after slotUnifyOutputs" << slotTimeDelay << mKdsStatus;
        }
        showBrightnessFrame();
    });
    initMultScreenStatus();

    bool isExtendMode = (status == 2);
    if (isExtendMode) {
        mScreen->setCloseOutput(false);
        mScreen->setExtendMode(true);
        qDebug() << Q_FUNC_INFO << "set close output false";
    } else {
        mScreen->setExtendMode(false);
    }

    if (m_StatusDbus->isValid()) {
        QDBusReply<bool> is_tabletmode = m_StatusDbus->call("get_current_tabletmode");
        if (is_tabletmode) {
            mMultiScreenCombox->setEnabled(false);
        } else {
            mMultiScreenCombox->setEnabled(true);
        }
    }
}

void Widget::initConnection()
{
    connect(monitorBtn, &QPushButton::clicked, this, [=](bool status){
        primaryButtonEnable(status);
        delayApply();
    });

    // Intel隐藏分辨率等调整选项
    if (UkccCommon::isTablet()) {
        mControlPanel->setVisible(false);
        scaleFrame->setVisible(false);
    }

    connect(mControlPanel, &ControlPanel::toSetScreenPos, this, [=](const KScreen::OutputPtr &output){
        for (QMLOutput *qmlOutput : mScreen->outputs()) {
            if (output && qmlOutput->output() == output) {
                qmlOutput->currentOutputSizeChanged(); //触发qml修改长和宽
                qmlOutput->setAllowResetSize(false);   //使save时不再去修改qml的长和宽，save后复位
                qmlOutput->updateRootProperties();
                qDebug() << Q_FUNC_INFO << "rotation status" << mControlPanel->getRotationStatus() << changeItm;
                KScreen::Output::Rotation rotation = mControlPanel->getRotationStatus();
                if (changeItm == 2 && (rotation == KScreen::Output::Left || rotation == KScreen::Output::Right)) {
                    mScreen->setRotation(true);
                    qDebug() << Q_FUNC_INFO << "mscreen set rotation true";
                }
                qDebug() << "toSetScreenPos before setScreenPos";
                mScreen->setScreenPos(qmlOutput, false);
            }
        }
    });

    connect(mControlPanel, &ControlPanel::changed, this, &Widget::changed);
    connect(this, &Widget::changed, this, [=]() {
            changedSlot();
            delayApply();
    });
    connect(mControlPanel, &ControlPanel::scaleChanged, this, &Widget::changescale);
    connect(mControlPanel, &ControlPanel::enabledChanged, this, &Widget::outputEnableChanged);

    connect(mMultiScreenCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        setMultiScreenSlot(mMultiScreenCombox->itemData(index).toString());
        UkccCommon::buriedSettings(QString("display"), QString("mMultiScreenCombox"), QString("select"), QString::number(index));
    });

    QDBusConnection::sessionBus().connect(QString("org.ukui.SettingsDaemon"),
                                          QString("/org/ukui/SettingsDaemon/xrandr"),
                                          QString("org.ukui.SettingsDaemon.xrandr"),
                                          "screenModeChanged",
                                          this,
                                          SLOT(usdScreenModeChangedSlot(int)));

    mOutputTimer = new QTimer(this);
    connect(mOutputTimer, &QTimer::timeout,
            this, &Widget::clearOutputIdentifiers);

    mApplyShortcut = new QShortcut(QKeySequence("Ctrl+A"), this);
    connect(mApplyShortcut, SIGNAL(activated()), this, SLOT(save()));

    connect(monitorComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [=](int index) {
        mainScreenButtonSelect(index);
        showBrightnessFrame();  //当前屏幕框变化的时候，显示，此时不判断
    });

    connect(scaleComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [=](int index){
        scaleChangedSlot(scaleComboBox->itemData(index).toDouble());
        UkccCommon::buriedSettings(QString("display"), QString("screen zoom"), QString("select"), scaleComboBox->currentText());
    });
    connect(scaleGSettings,&QGSettings::changed,this,[=](QString key){
        if (!key.compare("scalingFactor", Qt::CaseSensitive)) {
            double scale = scaleGSettings->get(key).toDouble();
            if (scaleComboBox->findData(scale) == -1) {
                scale = 1.0;
            }
            scaleComboBox->blockSignals(true);
            scaleComboBox->setCurrentText(QString::number(scale * 100) + "%");
            scaleComboBox->blockSignals(false);
        }
    });

    connect(m_primaryCheckBox, &QCheckBox::clicked, this, [=](bool checked) {
        primaryCheckBoxClickSlot(checked);
    });

    setEnableBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setEnableBySecurity()));
}

void Widget::initNightModeConnection()
{
    connect(m_openTimeHCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]{
        openTimeHComboxIdxChangeSlot();
    });

    connect(m_openTimeMCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]{
        openTimeMComboxIdxChangeSlot();
    });

    connect(m_closeTimeHCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]{
        closeTimeHComboxIdxChangeSlot();
    });

    connect(m_closeTimeMCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]{
        closeTimeMComboxIdxChangeSlot();
    });

    connect(m_tempSlider, &QSlider::valueChanged, this, &Widget::tempSliderChangedSlot);
    connect(m_tempSlider, &QSlider::sliderReleased, this, &Widget::tempSliderReleasedSlot);

    connect(m_timeModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](){
        timeModeComboBoxIdxChangeSlot();
    });
    connect(m_defaultRadioButton, &QRadioButton::clicked, this, [=] {
        defaultRadioButtonClickSlot();
    });
    connect(m_customRadioButton, &QRadioButton::clicked, this, [=] {
        customRadioButtonClickSlot();
    });
    connect(m_eyesModeSwitchWidget->switchButton(), &KSwitchButton::stateChanged, [=](bool checked) {
        if (m_colorSettings->keys().contains("mainSwitch")) {
            m_colorSettings->set(MAIN_SWITCH, checked);
        }

        QTimer::singleShot(100, this, [=, checked] {
            this->setUpdatesEnabled(false);
            eyesModeFrameSlot(checked);
            this->setUpdatesEnabled(true);
            this->update();
            qDebug() << Q_FUNC_INFO << "update eyesModeFrameSlot";
        });
    });
}

void Widget::eyesModeFrameSlot(bool checked)
{
    qDebug() << Q_FUNC_INFO << "eyesModeFrameSlot status" << checked;

    if (checked) {
        m_eyesModeSwitchWidget->switchButton()->setChecked(true);
        m_defaultNightModeFrame->setVisible(true);
        m_customNightModeFrame->setVisible(true);
        if (m_colorSettings->keys().contains("eyeCare")) {
            m_isEyeCareMode = m_colorSettings->get(EYES_MODE).toBool();
        }
        m_isNightMode = m_colorSettings->get(NIGHT_ENABLE_KEY).toBool();
        qDebug() << Q_FUNC_INFO << "isEyeCareMode" << m_isEyeCareMode << "isNightMode" << m_isNightMode;
        if (!m_isNightMode) {
            m_defaultRadioButton->setChecked(true);
            m_customRadioButton->setChecked(false);

            m_tempFrame->setVisible(false);
            m_timeModeFrame->setVisible(false);
            m_customTimeFrame->setVisible(false);
        } else {
            m_defaultRadioButton->setChecked(false);
            m_customRadioButton->setChecked(true);

            m_tempFrame->setVisible(true);
            m_timeModeFrame->setVisible(true);
            if (m_timeModeComboBox->currentIndex() == 2) {
                m_customTimeFrame->setVisible(true);
            } else {
                m_customTimeFrame->setVisible(false);
            }
        }
    } else {
        m_eyesModeSwitchWidget->switchButton()->setChecked(false);
        m_defaultNightModeFrame->setVisible(false);
        m_customNightModeFrame->setVisible(false);
        m_tempFrame->setVisible(false);
        m_timeModeFrame->setVisible(false);
        m_customTimeFrame->setVisible(false);
    }
    UkccCommon::buriedSettings(QString("display"), QString("Eye Protection Mode"), QString("clicked"), UkccCommon::boolToString(checked));
}

void Widget::primaryCheckBoxClickSlot(bool checked) {
    m_primaryCheckBox->setEnabled(false);
    primaryButtonEnable(checked);
    delayApply();
}

void Widget::setNightComponent()
{
    /* 设置时间模式 */
    mTimeModeStringList << tr("All Day") << getSunTime() << tr("Custom Time");
    m_timeModeComboBox->insertItem(0, mTimeModeStringList.at(0));
    m_timeModeComboBox->insertItem(1, mTimeModeStringList.at(1));
    m_timeModeComboBox->insertItem(2, mTimeModeStringList.at(2));

    for (int i = 0; i < 24; i++) {
        m_openTimeHCombox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
        m_closeTimeHCombox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
    }

    for (int i = 0; i < 60; i++) {
        m_openTimeMCombox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
        m_closeTimeMCombox->addItem(QStringLiteral("%1").arg(i, 2, 10, QLatin1Char('0')));
    }
}

QFrame *Widget::setLine(QFrame *frame)
{
    QFrame *line = new QFrame(frame);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

void Widget::setNightMode(const bool nightMode)
{
    QDBusInterface colorIft("org.ukui.KWin",
                            "/ColorCorrect",
                            "org.ukui.kwin.ColorCorrect",
                            QDBusConnection::sessionBus());
    if (!colorIft.isValid()) {
        qWarning() << "create org.ukui.kwin.ColorCorrect failed";
        return;
    }

    if (!nightMode) {
        mNightConfig["Active"] = false;
    } else {
        mNightConfig["Active"] = true;
        if (m_timeModeComboBox->currentIndex() == 1) {
            mNightConfig["EveningBeginFixed"] = "17:55:01";
            mNightConfig["MorningBeginFixed"] = "06:23:00";
            mNightConfig["Mode"] = 2;
        } else if (m_timeModeComboBox->currentIndex() == 2) {
            mNightConfig["EveningBeginFixed"] = m_openTimeHCombox->currentText() + ":"
                                                + m_openTimeMCombox->currentText() + ":00";
            mNightConfig["MorningBeginFixed"] = m_closeTimeHCombox->currentText() + ":"
                                                + m_closeTimeMCombox->currentText() + ":00";
            mNightConfig["Mode"] = 2;
        } else if (m_timeModeComboBox->currentIndex() == 0) {
             mNightConfig["Mode"] = 3;
        }
        mNightConfig["NightTemperature"] = m_tempSlider->value();
    }

    colorIft.call("setNightColorConfig", mNightConfig);
}

void Widget::initUiComponent()
{
    mDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
            %QStringLiteral("/kscreen/")
            %QStringLiteral("" /*"configs/"*/);

    mScaleDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
            %QStringLiteral("/kscreen/scale/");

    mIsBattery = isBacklight();

    m_isPanguX  = isPanguX();
    m_isCe72    = isCe72();
    m_isSetGM  = isSetGammaBrightness();
    m_isSupportGM = isSupoortGammaBrighnes();

    mUPowerInterface = QSharedPointer<QDBusInterface>(
        new QDBusInterface("org.freedesktop.UPower",
                           "/org/freedesktop/UPower",
                           "org.freedesktop.DBus.Properties",
                           QDBusConnection::systemBus()));

    if (!mUPowerInterface.get()->isValid()) {
        qDebug() << "Create UPower Battery Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return;
    }

    QDBusReply<QVariant> batteryInfo;
    batteryInfo = mUPowerInterface.get()->call("Get", "org.freedesktop.UPower", "OnBattery");
    if (batteryInfo.isValid()) {
        mOnBattery = batteryInfo.value().toBool();
    }

    mUPowerInterface.get()->connection().connect("org.freedesktop.UPower",
                                                 "/org/freedesktop/UPower",
                                                 "org.freedesktop.DBus.Properties",
                                                 "PropertiesChanged",
                                                 this,
                                                 SLOT(propertiesChangedSlot(QString, QMap<QString,QVariant>, QStringList)));

    mUkccInterface = QSharedPointer<QDBusInterface>(
                new QDBusInterface("org.ukui.ukcc.session",
                                   "/",
                                   "org.ukui.ukcc.session.interface",
                                   QDBusConnection::sessionBus()));
}

void Widget::hideComponent()
{
    if ((UkccCommon::isOpenkylin() || UkccCommon::isV11()) && UkccCommon::isWayland()) {
         scaleFrame->hide();
    }
    if (!m_nightModeFrame || !m_nightModeLabel || !m_eyesModeSwitchWidget) {
        return;
    }
    m_nightModeFrame->setVisible(QString(QLatin1String(kdk_system_get_hostVirtType())) == "none");
    m_nightModeLabel->setVisible(QString(QLatin1String(kdk_system_get_hostVirtType())) == "none");

    if (!m_isSupportGM) {
        m_nightModeFrame->setVisible(false);
        m_nightModeLabel->setVisible(false);
        m_eyesModeSwitchWidget->setVisible(false);
        qDebug() << Q_FUNC_INFO << "not support gamma, hide eye model";
    }
}

void Widget::initNightStatus()
{
    setNightModeSetting();
}

void Widget::setNightModeSetting()
{
    if (!m_colorSettings) {
        applyNightModeSlot();
        return;
    }
    if (m_colorSettings->keys().contains("eyeCare")) {
        m_isEyeCareMode = m_colorSettings->get(EYES_MODE).toBool();
    }
    m_isNightMode = m_colorSettings->get(NIGHT_ENABLE_KEY).toBool();
    bool mainSwitch = true;
    if (m_colorSettings->keys().contains("mainSwitch")) {
        mainSwitch = m_colorSettings->get(MAIN_SWITCH).toBool();
    }
    qDebug() << Q_FUNC_INFO << "isEyeCareMode" << m_isEyeCareMode << "isNightMode" << m_isNightMode << "main-switch" << mainSwitch;
    m_timeModeComboBox->blockSignals(true);
    if (m_isNightMode) {
        if(m_colorSettings->get(AllDAY_KEY).toBool()) {
            m_timeModeComboBox->setCurrentIndex(0);
        } else if(m_colorSettings->get(AUTO_KEY).toBool()) {
            m_timeModeComboBox->setCurrentIndex(1);
        } else {
            m_timeModeComboBox->setCurrentIndex(2);
            double openTime = m_colorSettings->get(NIGHT_FROM_KEY).toDouble();
            double cltime = m_colorSettings->get(NIGHT_TO_KEY).toDouble();
            int ophour, opmin, clhour, clmin;
            value_to_hour_minute(openTime, &ophour, &opmin);
            value_to_hour_minute(cltime, &clhour, &clmin);

            m_openTimeHCombox->setCurrentIndex(ophour);
            m_openTimeMCombox->setCurrentIndex(opmin);
            m_closeTimeHCombox->setCurrentIndex(clhour);
            m_closeTimeMCombox->setCurrentIndex(clmin);
            qDebug() << Q_FUNC_INFO << "set timeMode" << ophour << ":" << opmin << "to" << clhour << ":" << clmin;
        }
        int value = m_colorSettings->get(NIGHT_TEMPERATURE_KEY).toInt();
        m_tempSlider->setValue(value);
        qDebug() << Q_FUNC_INFO << "get night-light-temperature" << value;
    }
    m_timeModeComboBox->blockSignals(false);
    if (mainSwitch && (m_isEyeCareMode || m_isNightMode)) {
        m_eyesModeSwitchWidget->switchButton()->setChecked(true);
        eyesModeFrameSlot(true);
        qDebug() << Q_FUNC_INFO << "open EyeCareMode";
    } else {
        m_eyesModeSwitchWidget->switchButton()->setChecked(false);
        eyesModeFrameSlot(false);
        qDebug() << Q_FUNC_INFO << "close EyeCareMode";
    }
}

QString Widget::getSunTime()
{
    if (m_colorSettings) {
        if(m_colorSettings->keys().contains("nightLightScheduleAutomaticFrom")
                && m_colorSettings->keys().contains("nightLightScheduleAutomaticTo")) {
            double openTime = m_colorSettings->get(AUTO_NIGHT_FROM_KEY).toDouble();
            double cltime = m_colorSettings->get(AUTO_NIGHT_TO_KEY).toDouble();
            int ophour, opmin, clhour, clmin;
            value_to_hour_minute(openTime, &ophour, &opmin);
            value_to_hour_minute(cltime, &clhour, &clmin);
            QVector<QString> timevector;
            QVector<int> res;
            res << ophour << opmin << clhour << clmin;
            qDebug() << Q_FUNC_INFO << ophour << opmin << clhour << clmin;
            for (int i : res)
                timevector.append(QString("%1%2").arg(i < 10 ? "0" : "").arg(QString::number(i)));

            return QString("%1 (%2)").arg(tr("Follow the sunrise and sunset"))
                                    .arg(QString("%1:%2-%3:%4").arg(timevector.at(0))
                                    .arg(timevector.at(1)).arg(timevector.at(2))
                                    .arg(timevector.at(3)));
        }
    }
    return tr("Follow the sunrise and sunset");
}


/* 总结: 亮度条怎么显示和实际的屏幕状态有关,与按钮选择状态关系不大:
 * 实际为镜像模式，就显示所有屏幕的亮度(笔记本外显除外，笔记本外显任何情况均隐藏，这里未涉及)。
 * 实际为扩展模式，就显示当前选中的屏幕亮度，如果当前选中复制模式，则亮度条隐藏不显示，应用之后再显示所有亮度条;
 * 实际为单屏模式，即另一个屏幕关闭，则显示打开屏幕的亮度，关闭的显示器不显示亮度
 *
 *ps: by feng chao
*/

void Widget::showBrightnessFrame(const int flag)
{
    Q_UNUSED(flag);
    bool allShowFlag = true;
    allShowFlag = mIscloneMode;
    // 双屏及以下保持原提前退出；三屏及以上必须更新全部亮度条（如 DP/HDMI 后再插 VGA）。
    const bool needScanAll = mConfig->connectedOutputs().count() >= 3;
    int enabledOutputsCount = 0;
    Q_FOREACH (const KScreen::OutputPtr &output, mConfig->outputs()) {
        for (int i = 0; i < BrightnessFrameV.size(); ++i) {
            if (BrightnessFrameV[i]->getOutputName() == UtilsOfDisplay::outputName(output)) {
                const bool active = mIscloneMode ? output->isConnected() : output->isEnabled();
                BrightnessFrameV[i]->setOutputEnable(active);
                break;
            }
        }
        if (!needScanAll) {
            if (output->isEnabled()) {
                ++enabledOutputsCount;
            }
            if (enabledOutputsCount > 1) {
                break;
            }
        }
    }
    if (allShowFlag == true) { //镜像模式/即将成为镜像模式
        for (int i = 0; i < BrightnessFrameV.size(); ++i) {
            if (!BrightnessFrameV[i]->getOutputEnable()) {
                BrightnessFrameV[i]->setVisible(false);
                continue;
            }
            BrightnessFrameV[i]->setOutputEnable(true);
            //~ contents_path /Display/Brightness
            BrightnessFrameV[i]->setTextLabelName(tr("Brightness") + QString("(") + BrightnessFrameV[i]->getOutputName() + QString(")"));
            BrightnessFrameV[i]->setVisible(true);
        }
    } else {
        for (int i = 0; i < BrightnessFrameV.size(); ++i) {
            if (monitorComboBox->currentText() == BrightnessFrameV[i]->getOutputName() && BrightnessFrameV[i]->getOutputEnable()) {
                BrightnessFrameV[i]->setTextLabelName(tr("Brightness"));
                BrightnessFrameV[i]->setVisible(true);
                //不能break，要把其他的frame隐藏
            } else {
                BrightnessFrameV[i]->setVisible(false);
            }
        }
    }
    if (mMultiScreenCombox->currentIndex() != SPLICE) {
        mbrightnessFrame->setVisible(true);
    } else {
        mbrightnessFrame->setVisible(false);
    }
    configGroup->updateShape();
}

void Widget::changescale(QSize size)
{
    if (UkccCommon::isTablet() || ((UkccCommon::isOpenkylin() || UkccCommon::isV11()) && UkccCommon::isWayland())) return;

    mScaleSizeRes = QSize();
    if (mIscloneMode && size != QSize()) {
        mScaleSizeRes = size;
    } else {
        for (const KScreen::OutputPtr &output : mConfig->outputs()) {
            if (output->isEnabled()) {
                // 作判空判断，防止控制面板闪退
                if (output->currentMode()) {
                    if (mScaleSizeRes == QSize()) {
                        mScaleSizeRes = output->currentMode()->size();
                    } else {
                        mScaleSizeRes = mScaleSizeRes.width() < output->currentMode()->size().width()?mScaleSizeRes:output->currentMode()->size();
                    }
                } else {
                    return;
                }
            }
        }
    }

    if (mScaleSizeRes != QSize(0,0)) {
        QSize scalesize = mScaleSizeRes;
        scaleComboBox->blockSignals(true);
        scaleComboBox->clear();
        scaleComboBox->addItem("100%", 1.0);

        if (scalesize.width() > 1024 ) {
            scaleComboBox->addItem("125%", 1.25);
        }
        if (scalesize.width() == 1920 ) {
            scaleComboBox->addItem("150%", 1.5);
        }
        if (scalesize.width() > 1920) {
            scaleComboBox->addItem("150%", 1.5);
            scaleComboBox->addItem("175%", 1.75);
        }
        if (scalesize.width() >= 2160) {
            scaleComboBox->addItem("200%", 2.0);
        }
        if (scalesize.width() > 2560) {
            scaleComboBox->addItem("225%", 2.25);
        }
        if (scalesize.width() > 3072) {
            scaleComboBox->addItem("250%", 2.5);
        }
        if (scalesize.width() > 3840) {
            scaleComboBox->addItem("275%", 2.75);
        }

        double scale;
        QStringList keys = scaleGSettings->keys();
        if (keys.contains("scalingFactor")) {
            scale = scaleGSettings->get(SCALE_KEY).toDouble();
        }

        if (scaleComboBox->findData(scale) == -1) {
            //记录分辨率切换时，新分辨率不存在的缩放率，在用户点击恢复设置时写入
            mIsSCaleRes = true;

            //记录是否因分辨率导致的缩放率变化
            mIsChange = true;

            scaleres = scale;
            scale = 1.0;
        }
        scaleComboBox->setCurrentText(QString::number(scale * 100) + "%");
        scaleChangedSlot(scale);
        scaleComboBox->blockSignals(false);

        mScaleSizeRes = QSize();
    }
}

void Widget::mOutputClicked() {
     if (mIscloneMode || mConfig->connectedOutputs().count() < 2) {
         return; //镜像模式以及显示器小于2则不检测
     }
    QMLOutput *mOutput = qobject_cast<QMLOutput *>(sender());
    mOutputClickedName = mOutput->output()->name();
    slotIdentifyButtonClicked(true);
}

bool Widget::isBacklightAllInOne()
{
    QString cmd = "";
    QByteArray buf;
    QProcess process;
    QFile file("/proc/cpuinfo");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<file.errorString();
    }
    buf = file.readAll();
    file.close();
    cmd = "cat /sys/class/backlight/*/max_brightness";
    //来自ukui-power-manager
    //220项目中D2000使用 cmd = "cat /sys/class/backlight/ec_bl/max_brightness";无法获取，先注释
//    if (-1 == buf.indexOf("ZHAOXIN")) {
//        if (-1 == buf.indexOf("D2000")) {
//            cmd = "cat /sys/class/backlight/*/max_brightness";
//        } else {
//            cmd = "cat /sys/class/backlight/ec_bl/max_brightness";
//        }
//    }
//    else{
//        cmd = "cat /sys/class/backlight/acpi_video1/max_brightness";
//    }
    qDebug()<<"isBacklightAllInOne --- cmd:"<<cmd;
    process.start("bash", QStringList() <<"-c" << cmd);
    process.waitForFinished();
    QString strResult =process.readAllStandardOutput()+process.readAllStandardError();
    strResult = strResult.replace("\n", "");
    QString pattern("^[0-9]*$");
    QRegExp reg(pattern);
    return reg.exactMatch(strResult);
}

void Widget::getAllI2Cbus()
{
    I2CbusMap.clear();
    //暂时仅D2000机器直接使用i2c
    // 11th该机型也使用i2c,bug#154976,后续再出问题则考虑弃用ddc,暂特殊处理
    if (!(UkccCommon::getCpuInfo().contains("D2000", Qt::CaseInsensitive) ||
          UkccCommon::getCpuInfo() == QString("11th Gen Intel(R) Core(TM) i5-1135G7 @ 2.40GHz"))) {
        return;
    }

    getEdidI2Cbus();
    if (edidI2CbusMap.isEmpty()) {
        QMap<QString, QString> msg;
        QString cmd = "find /sys/class/drm/card0-*/*/ -name '*i2c-[0-9]*'";
        QProcess process;
        process.start("bash", QStringList() <<"-c"<<cmd);
        process.waitForFinished();
        QString strResult = process.readAllStandardOutput();
        QStringList resultList = strResult.split("\n"); //所有结果

        qInfo()<<"read i2c process result = "<<resultList;
        for (int i = 0; i < resultList.size(); i++) {
            QStringList i2cList = resultList.at(i).split("/"); //切割每一行结果
            if (i2cList.size() >= 5) {
                if (!msg.keys().contains(i2cList.at(4))) {
                    //当不存在时就添加
                    msg.insert(i2cList.at(4), resultList.at(i));
                } else {
                    //已经存在，但是内容更少时也重新替换,drm中文件是嵌套的，可能存在包含关系，故做此处理
                    if (msg[i2cList.at(4)].size() > resultList.at(i).size()) {
                        msg[i2cList.at(4)] = resultList.at(i);
                    }
                }
            }
        }
        QMap<QString, QString>::const_iterator it;
        for (it = msg.constBegin(); it != msg.constEnd(); ++it) {
            qDebug()<<" ----------MAP-MSG--------- "<<it.key()<<" "<<it.value();
            //显示器名只取中间的(HDMI\VGA...)，因为后面的内容kscreen和内核提供的不一定一致
            //因此当有多个HDMI接口时此方式并不适用
            QString name = it.key().split("-").at(1);
            QStringList i2cList = it.value().split("/");
            QString i2cStr = i2cList.at(i2cList.size() - 1);
            if (!i2cStr.contains("i2c-",Qt::CaseInsensitive)) {
                i2cStr = i2cList.at(i2cList.size() - 2);
            }

            QString busNum = i2cStr.split("-").at(1);
            if (QString::number(busNum.toInt()) == busNum) {
                if (I2CbusMap.keys().contains(name)) {
                    qDebug()<<"Unable to get the correct bus number from the kernel ... "<<name;
                    I2CbusMap.clear(); //如果出现了一样的，表明有重复的接口，舍弃使用内核提供的bus号
                    return;
                }
                qInfo()<<" i2c-name = "<<name<<" *** "<<"i2c-bus="<<busNum;
                I2CbusMap.insert(name, busNum);
            }
        }
    }
    return;
}

// 获取所有显卡的节点信息
void Widget::getEdidI2Cbus()
{
    edidI2CbusMap.clear();
    QString cmd = "find /sys/class/drm/card*/*/ -name status";
    QProcess process;
    process.start("bash", QStringList() <<"-c"<<cmd);
    process.waitForFinished();
    QString strResult = process.readAllStandardOutput();
    QStringList resultList = strResult.split("\n"); //所有结果
    resultList.removeOne("");

    QMap<QString, QString> screenMap;
    for (QString s : resultList) {
        s.replace("/status", "");
        QStringList sl = s.split("/");
        if (sl.size() >= 1) {
            QString screenCardName = sl.at(sl.size() - 1);
            if (!screenMap.keys().contains(screenCardName)) {
                screenMap.insert(screenCardName, s);
            }
        }
    }
    qInfo()<<"screenMap:"<<screenMap;
    for (QMap<QString, QString>::const_iterator it = screenMap.constBegin(); it != screenMap.constEnd(); ++it) {
        QString fileName = it.value() + QString("/status");
        QFile statusFile(fileName);
        if (statusFile.exists() && statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString str(statusFile.readAll());
            str.replace("\n", "");
            statusFile.close();
            // 是已经连接的显示器
            if (str == "connected") {
                qInfo() << "connected screen:" << it.value();
                fileName = it.value() + QString("/edid");
                QFile edidFile(fileName);
                if (edidFile.exists() && edidFile.open(QIODevice::ReadOnly)) {
                    QByteArray edidByteArray = edidFile.readAll();
                    edidFile.close();
                    QCryptographicHash hash(QCryptographicHash::Md5);
                    hash.reset();
                    const quint8 *edidData = reinterpret_cast<const quint8 *>(edidByteArray.constData());
                    hash.addData(reinterpret_cast<const char *>(edidData), 128);
                    QString edidHash = QString::fromLatin1(hash.result().toHex());
                    QString path1 = it.value() + QString("/");
                    QString path2 = it.value() + QString("/ddc/i2c-dev/");
                    QString cmdStr = QString("find ") + path1 + QString(" ") + path2 + QString(" -maxdepth 1") + QString(" -name '*i2c-[0-9]*'");
                    process.start("bash", QStringList() <<"-c"<<cmdStr);
                    process.waitForFinished();
                    strResult = process.readAllStandardOutput();
                    qDebug()<<"cmdStr:"<<cmdStr<<"resultStr:"<<strResult;
                    // 获取第一行，理论上只有一行
                    QString i2cStr = strResult.split("\n").at(0);
                    QStringList i2cStrList = i2cStr.split("/");
                    QStringList busStrList =  i2cStrList.at(i2cStrList.size() - 1).split("-");
                    if (busStrList.size() > 1) {
                        QString busNum = busStrList.at(busStrList.size() - 1);
                        if (QString::number(busNum.toInt()) == busNum && !edidI2CbusMap.keys().contains(edidHash)) {
                            edidI2CbusMap.insert(edidHash, busNum);
                        }
                    }
                }
            }
        }
    }
    qDebug() << "edidI2CbusMap:"<<edidI2CbusMap;
}

bool Widget::isSetGammaBrightness()
{
    QDBusInterface upower("org.ukui.powermanagement",
                          "/",
                          "org.ukui.powermanagement.interface",
                          QDBusConnection::systemBus());
    if (!upower.isValid()) {
        qDebug() << "Create UPower Interface Failed : " << QDBusConnection::systemBus().lastError();
        return false;
    }
    QDBusReply<bool> reply = upower.call("CanSetBrightness");
    qDebug() << Q_FUNC_INFO << reply.value() << reply.isValid() << mMachineType;
    if ((!reply.value() && (mMachineType == "all in one"))
            || (m_gsettings->keys().contains("gammaforbrightness") && m_gsettings->get("gammaforbrightness").toBool())
            || (!UkccCommon::getProductName().compare("VAH510"))) {
        return true;
    }
    return false;
}

bool Widget::isSupoortGammaBrighnes()
{
    QDBusInterface usdGlobalSignal("org.ukui.SettingsDaemon",
                                   "/GlobalSignal",
                                   "org.ukui.SettingsDaemon.GlobalSignal",
                                   QDBusConnection::sessionBus());
    if (usdGlobalSignal.isValid()) {
        QDBusReply<bool> gamma = usdGlobalSignal.call("isSupportGamma");
        qDebug() << Q_FUNC_INFO << gamma.isValid() << gamma.error().type();
        if (!gamma.isValid() || (gamma.error().type() == QDBusError::ErrorType::UnknownMethod)) {
            return true;
        } else {
            return gamma.value();
        }
    }
    return true;
}

void Widget::initUi()
{
    modeGroup         = new SettingGroup(this);
    mMultiScreenFrame = new UkccFrame(modeGroup, UkccFrame::BorderRadiusStyle::Around, true);
    configGroup       = new SettingGroup(this);
    monitorFrame      = new UkccFrame(configGroup, UkccFrame::BorderRadiusStyle::Around, true);
    monitorLabel      = new KLabel(monitorFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(monitorLabel, "Display", "", "");
    monitorComboBox   = new QComboBox(monitorFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(monitorComboBox, "Display", "", "");
    monitorBtn        = new QPushButton(monitorFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(monitorBtn, "Display", "", "");
    mControlPanel     = new ControlPanel(configGroup);
    scaleFrame        = new UkccFrame(configGroup, UkccFrame::BorderRadiusStyle::Around, true);
    scaleLabel        = new KLabel(scaleFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(scaleLabel, "Display", "", "");
    scaleComboBox     = new QComboBox(scaleFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(scaleComboBox, "Display", "", "");
    openMonitorFrame  = new UkccFrame(configGroup, UkccFrame::BorderRadiusStyle::Around, true);
    openMonitorLabel  = new KLabel(openMonitorFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(openMonitorLabel, "Display", "", "");
    // openMonitorButton 延后到 OutputConfig 缩放创建之后再建，见 initOpenMonitorButton
    //~ contents_path /Display/Auto Brightness
    mAutoBrightFrame   = new SwitchWidget(tr("Auto Brightness"));
    mbrightnessFrame  = new UkccFrame(configGroup);
    m_enhanceBrightFrame = new UkccFrame(configGroup);
    mbrightnesslayout = new QVBoxLayout(mbrightnessFrame);

    ui->modeLayout->addWidget(modeGroup);
    modeGroup->addWidget(mMultiScreenFrame);
    ui->configLayout->addWidget(configGroup);
    configGroup->addWidget(monitorFrame);
    configGroup->addWidget(mControlPanel);
    configGroup->addWidget(scaleFrame);
    configGroup->addWidget(openMonitorFrame);
    configGroup->addWidget(mbrightnessFrame);
    configGroup->addWidget(m_enhanceBrightFrame);
    configGroup->addWidget(mAutoBrightFrame);

#ifndef Nile
    mAutoBrightFrame->setVisible(false);
#endif

    monitorFrame->setVisible(false);
    QHBoxLayout *monitorLayout = new QHBoxLayout(monitorFrame);
    monitorLayout->setContentsMargins(16, 0, 16, 0);
    monitorLayout->setSpacing(16);
    monitorLayout->addWidget(monitorLabel);
    monitorLayout->addWidget(monitorComboBox);
    monitorLayout->addWidget(monitorBtn);
    monitorLabel->setFixedWidth(108);
    monitorBtn->setMinimumWidth(130);
    monitorBtn->setMaximumWidth(150);
    monitorLabel->setText(tr("Monitor"));
    monitorBtn->setText(tr("As Main"));

    QHBoxLayout *scaleLayout = new QHBoxLayout(scaleFrame);
    scaleLayout->setContentsMargins(16, 0, 16, 0);
    scaleLayout->setSpacing(16);
    scaleLayout->addWidget(scaleLabel);
    scaleLayout->addWidget(scaleComboBox);
    scaleLabel->setFixedWidth(108);
    scaleLabel->setText(tr("Screen Zoom"));

    QHBoxLayout *openMonitorLayout = new QHBoxLayout(openMonitorFrame);
    openMonitorLayout->setContentsMargins(16, 0, 16, 0);
    openMonitorLabel->setFixedWidth(108);
    openMonitorLayout->addWidget(openMonitorLabel);
    openMonitorLayout->addStretch();
    openMonitorLabel->setText(tr("Open Monitor"));

    mbrightnessFrame->setContainer(true);
    mbrightnesslayout->setMargin(0);
    mbrightnesslayout->setSpacing(1);

    m_enhanceBrightFrame->setFrameShape(QFrame::Shape::Box);
    m_enhanceBrightFrame->setMinimumSize(550, 60);
    m_enhanceBrightFrame->setMaximumSize(16777215, 60);

    QHBoxLayout *enhanceBrightHLayout = new QHBoxLayout(m_enhanceBrightFrame);
    enhanceBrightHLayout->setContentsMargins(16, 0, 16, 0);

    //~ contents_path /Display/Brightness Enhancement
    m_enhanceBrightLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_enhanceBrightLabel, "Display", "", "");
    m_enhanceBrightLabel->setText(tr("Brightness Enhancement"));
    m_enhanceBrightBtn   = new KSwitchButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_enhanceBrightBtn, "Display", "", "");
    enhanceBrightHLayout->addWidget(m_enhanceBrightLabel);
    enhanceBrightHLayout->addStretch();
    enhanceBrightHLayout->addWidget(m_enhanceBrightBtn);

    m_enhanceBrightFrame->setLayout(enhanceBrightHLayout);

    m_primaryFrame = new UkccFrame(m_primaryGroup, UkccFrame::BorderRadiusStyle::Around, true);
    m_primaryCheckBox = new QCheckBox(tr("Set As Main"), m_primaryFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_primaryCheckBox, "Display", "", "");

    QHBoxLayout *primaryLayout = new QHBoxLayout(m_primaryFrame);
    primaryLayout->setContentsMargins(16, 0, 16, 0);
    primaryLayout->addStretch();
    primaryLayout->addWidget(m_primaryCheckBox);
    primaryLayout->addStretch();

    m_primaryGroup = new SettingGroup(this);
    m_primaryGroup->addWidget(m_primaryFrame);

    m_primaryLine = setLine(m_primaryFrame);

    ui->mainframe->layout()->addWidget(m_primaryLine);
    ui->mainframe->layout()->addWidget(m_primaryGroup);

    monitorLabel->setAccessibleName(tr("Monitor"));
    monitorLabel->setBuddy(monitorComboBox);
    monitorComboBox->setAccessibleName(tr("Monitor"));
    monitorBtn->setAccessibleName(tr("As Main"));
    scaleLabel->setAccessibleName(tr("Screen Zoom"));
    scaleLabel->setBuddy(scaleComboBox);
    scaleComboBox->setAccessibleName(tr("Screen Zoom"));
    openMonitorLabel->setAccessibleName(tr("Open Monitor"));
    m_enhanceBrightBtn->setAccessibleName(tr("Brightness Enhancement"));
    m_primaryCheckBox->setAccessibleName(tr("Set As Main"));
    // 预览 Tab 出链后落到「设为主显示器」，与视觉顺序一致
    QWidget::setTabOrder(ui->quickWidget, m_primaryCheckBox);
}

void Widget::initOpenMonitorButton()
{
    if (!openMonitorFrame) {
        return;
    }
    // 保留状态：ControlPanel 重建 OutputConfig 后需重排 Tab 链
    const bool wasChecked = openMonitorButton ? openMonitorButton->isChecked() : true;
    const bool wasEnabled = openMonitorButton ? openMonitorButton->isEnabled() : true;
    if (openMonitorButton) {
        delete openMonitorButton;
        openMonitorButton = nullptr;
    }
    openMonitorButton = new KSwitchButton(openMonitorFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(openMonitorButton, "Display", "", "");
    openMonitorButton->setAccessibleName(tr("Open Monitor"));
    openMonitorButton->setFocusPolicy(Qt::StrongFocus);
    openMonitorButton->blockSignals(true);
    openMonitorButton->setChecked(wasChecked);
    openMonitorButton->setEnabled(wasEnabled);
    openMonitorButton->blockSignals(false);
    if (QHBoxLayout *layout = qobject_cast<QHBoxLayout *>(openMonitorFrame->layout())) {
        layout->addWidget(openMonitorButton);
    }
    connect(openMonitorButton, &KSwitchButton::stateChanged,
            this, [=](bool checked) {
        mScreen->setCloseOutput(true);
        checkOutputScreen(checked);
        changescale();
    });
}

void Widget::alignScaleToOpenMonitorTabOrder()
{
    if (!openMonitorButton || !openMonitorButton->isVisibleTo(this)) {
        return;
    }
    // Wayland 可见缩放在当前 OutputConfig；X11 用页面 scaleComboBox
    QWidget *scaleWidget = nullptr;
    const QList<QComboBox *> wayScales =
            findChildren<QComboBox *>(QLatin1String("scaleCombox"));
    for (QComboBox *wayScale : wayScales) {
        if (wayScale && wayScale->isVisibleTo(this)) {
            scaleWidget = wayScale;
            break;
        }
    }
    if (!scaleWidget && scaleComboBox && scaleComboBox->isVisibleTo(this)) {
        scaleWidget = scaleComboBox;
    }
    if (!scaleWidget) {
        return;
    }
    // OutputConfig 会随 setConfig/切屏销毁或切换，需显式接上「缩放→打开显示器」
    QWidget::setTabOrder(scaleWidget, openMonitorButton);
}
