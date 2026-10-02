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
#include "outputconfig.h"
#include "resolutionslider.h"
#include "utils.h"
#include "scalesize.h"
#include "ukcccommon.h"
using namespace ukcc;

#include <QStringBuilder>
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>

#include <QComboBox>
#include <QGSettings>
#include <QDBusInterface>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/log.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/setconfigoperation.h>

#include <kysdk/kysdk-system/libkysysinfo.h>
#include <kysdk/applications/klineframe.h>

#include "combobox.h"
#include <klabel.h>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

double mScaleres = 0;
bool autoScaleChange = false;
QList<float> kRadeonRate{59.9402, 29.98};
CONFIG changeItm = INIT;
using namespace KScreen;
OutputConfig::OutputConfig(QWidget *parent) :
    UkccFrame(parent),
    mOutput(nullptr)
{
    this->setContainer(true);
    loadResolutionRefreshRateBlack();
    if (!m_usdDbus) {
        m_usdDbus = new QDBusInterface("org.ukui.SettingsDaemon",
                                       "/org/ukui/SettingsDaemon/xrandr",
                                       "org.ukui.SettingsDaemon.xrandr",
                                       QDBusConnection::sessionBus(), this);
    }
}

OutputConfig::OutputConfig(const KScreen::OutputPtr &output, QWidget *parent) :
    UkccFrame(parent)
{
    this->setContainer(true);
    if (!m_usdDbus) {
        m_usdDbus = new QDBusInterface("org.ukui.SettingsDaemon",
                                       "/org/ukui/SettingsDaemon/xrandr",
                                       "org.ukui.SettingsDaemon.xrandr",
                                       QDBusConnection::sessionBus(), this);
    }
    setOutput(output);
}

OutputConfig::~OutputConfig()
{
}

void OutputConfig::resolutionChangedSlot(QSize size, bool emitFlag) {
    qDebug() << Q_FUNC_INFO << "resolution changed" << size << "m_showPrefer" << m_showPrefer << "m_preferScale" << m_preferScale << mOutput->name();
    slotResolutionChanged(size, emitFlag);
    initScaleItem();
    if (m_showPrefer && m_preferIndex >= 0 && (m_preferIndex <= mScaleCombox->count())) {
        qDebug() << Q_FUNC_INFO << "resolutionChangedSlot changed scale" << mOutput->scale() << "to prefer scale" << m_preferScale << "prefer index" << m_preferIndex << mOutput->name() << mOutput->currentMode();
        mScaleCombox->blockSignals(true);
        mScaleCombox->setCurrentIndex(m_preferIndex);
        mScaleCombox->blockSignals(false);
        mOutput->setScale(m_preferScale);
    }

    UkccCommon::buriedSettings(QString("display"), QString("mResolution"), QString("select"), Utils::sizeToString(size));
}

void OutputConfig::setTitle(const QString &title)
{
    mTitle->setText(title);
}

void OutputConfig::initUi()
{
    GetConfigOperation op;
    op.exec();
    const ConfigPtr config = op.config();
    initConfig(config);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);

    // 分辨率下拉框
    mResolution = new ResolutionSlider(mOutput, this);

    QLabel *resLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(resLabel, "Display", "resolutionLabel", "resolution label of display");
    //~ contents_path /Display/Resolution
    resLabel->setText(tr("Resolution"));
    resLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    resLabel->setFixedWidth(108);

    QHBoxLayout *resLayout = new QHBoxLayout();
    resLayout->setContentsMargins(16, 0, 16, 0);
    resLayout->setSpacing(16);
    resLayout->addWidget(resLabel);
    resLayout->addWidget(mResolution);

    m_resFrame = new UkccFrame(this, UkccFrame::BorderRadiusStyle::None, true);
    m_resFrame->setLayout(resLayout);

    vbox->addWidget(m_resFrame);

    connect(mResolution, &ResolutionSlider::resolutionChanged,
            this, [=](QSize size, bool emitFlag) {
        resolutionChangedSlot(size, emitFlag);
    });

    // 方向下拉框
    mRotation = new QComboBox(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mRotation, "Display", "rotationCombox", "rotation combox of display");
    QLabel *rotateLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(rotateLabel, "Display", "rotationLabel", "rotation label of display");
    //~ contents_path /Display/Orientation
    rotateLabel->setText(tr("Orientation"));
    rotateLabel->setFixedWidth(108);

    QHBoxLayout *rotateLayout = new QHBoxLayout();
    rotateLayout->setContentsMargins(16, 0, 16, 0);
    rotateLayout->setSpacing(16);
    rotateLayout->addWidget(rotateLabel);
    rotateLayout->addWidget(mRotation);

    mRotateFrame = new UkccFrame(this, UkccFrame::BorderRadiusStyle::None, true);
    mRotateFrame->setLayout(rotateLayout);

    mRotation->addItem(tr("Arrow-Up"), KScreen::Output::None);
    mRotation->addItem(tr("90° Arrow-Right"), KScreen::Output::Right);
    mRotation->addItem(tr("90° Arrow-Left"), KScreen::Output::Left);
    mRotation->addItem(tr("Arrow-Down"), KScreen::Output::Inverted);
    connect(mRotation, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &OutputConfig::slotRotationChanged);
    mRotation->setCurrentIndex(mRotation->findData(mOutput->rotation()));
    KHLineFrame* rotationFrame = new KHLineFrame(mRotateFrame);
    vbox->addWidget(rotationFrame);
    vbox->addWidget(mRotateFrame);
    // 自动旋转
    mAutoRotation = new KSwitchButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mAutoRotation, "Display", "rotationLabel", "auto rotation button of display");

    QLabel *autoRotateLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(autoRotateLabel, "Display", "rotationLabel", "auto rotation label of display");
    //~ contents_path /Display/Auto Rotation
    autoRotateLabel->setText(tr("Auto Rotation"));
    autoRotateLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    autoRotateLabel->setFixedWidth(108);

    QHBoxLayout *autoRotateLayout = new QHBoxLayout();
    autoRotateLayout->setContentsMargins(16, 0, 16, 0);
    autoRotateLayout->addWidget(autoRotateLabel);
    autoRotateLayout->setSpacing(16);
    autoRotateLayout->addWidget(mAutoRotation);

    mAutoRotationFrame = new UkccFrame(this, UkccFrame::BorderRadiusStyle::None, true);
    mAutoRotationFrame->setLayout(autoRotateLayout);

    vbox->addWidget(mAutoRotationFrame);
    mAutoRotationFrame->setVisible(false);

    mStatusManager = new QDBusInterface("com.kylin.statusmanager.interface",
                                        "/",
                                        "com.kylin.statusmanager.interface",
                                        QDBusConnection::sessionBus(), this);

    if (mStatusManager->isValid()) {
        // 仅平板模式下，且硬件支持自动旋转才显示
        QDBusReply<bool> isSupportedAuto = mStatusManager->call("is_supported_autorotation");
        QDBusReply<bool> tabletmode = mStatusManager->call("get_current_tabletmode");
        qDebug() << "tabletmode = " << tabletmode << "isSupportedAuto = " << isSupportedAuto;
        if (tabletmode && isSupportedAuto) {
            mAutoRotationFrame->setVisible(true);
        }
        connect(mStatusManager, SIGNAL(mode_change_signal(bool)), this, SLOT(tabletChanged(bool)));

        QDBusReply<bool> isAutoRotation = mStatusManager->call("get_auto_rotation");
        mAutoRotation->setChecked(isAutoRotation);
        mRotation->setEnabled(!isAutoRotation);
        connect(mStatusManager, SIGNAL(auto_rotation_change_signal(bool)), this, SLOT(rotationDbusSlot(bool)));
        connect(mStatusManager, SIGNAL(rotations_change_signal(QString)), this, SLOT(rotationDirectionSlot(QString)));
    }

    connect(mAutoRotation, &KSwitchButton::stateChanged, this, [=](bool checked){
        mRotation->setEnabled(!checked);
        if (mStatusManager->isValid()) {
            mStatusManager->call("set_auto_rotation", checked, "ukcc", "set_auto_rotation");
        }
    });

    // 刷新率下拉框
    mRefreshRate = new QComboBox(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mRefreshRate, "Display", "refreshRateCombox", "refresh rate combox of display");
    QLabel *freshLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(freshLabel, "Display", "refreshRateLabel", "refresh rate label of display");
    //~ contents_path /Display/Frequency
    freshLabel->setText(tr("Frequency"));
    freshLabel->setFixedWidth(108);

    QHBoxLayout *freshLayout = new QHBoxLayout();
    freshLayout->setContentsMargins(16, 0, 16, 0);
    freshLayout->setSpacing(16);
    freshLayout->addWidget(freshLabel);
    freshLayout->addWidget(mRefreshRate);

    m_freshFrame = new UkccFrame(this, UkccFrame::BorderRadiusStyle::None, true);
    m_freshFrame->setLayout(freshLayout);
    KHLineFrame* refreshFrame = new KHLineFrame(m_freshFrame);
    vbox->addWidget(refreshFrame);
    vbox->addWidget(m_freshFrame);

    slotResolutionChanged(mResolution->currentResolution(), true);
    connect(mRefreshRate, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &OutputConfig::slotRefreshRateChanged);

    initConnection();

    resLabel->setAccessibleName(tr("Resolution"));
    mResolution->setBuddyLabel(resLabel);
    mResolution->setAccessibleName(tr("Resolution"));
    rotateLabel->setAccessibleName(tr("Orientation"));
    rotateLabel->setBuddy(mRotation);
    mRotation->setAccessibleName(tr("Orientation"));
    mAutoRotation->setAccessibleName(tr("Auto Rotation"));
    freshLabel->setAccessibleName(tr("Frequency"));
    freshLabel->setBuddy(mRefreshRate);
    mRefreshRate->setAccessibleName(tr("Frequency"));

    if (!((UkccCommon::isOpenkylin() || UkccCommon::isV11()) && UkccCommon::isWayland()))
        return;
    // 缩放率下拉框

    m_scaleFrame = new UkccFrame(this, UkccFrame::BorderRadiusStyle::None, true);

    QHBoxLayout *scaleLayout = new QHBoxLayout(m_scaleFrame);
    scaleLayout->setContentsMargins(16, 0, 16, 0);

    mScaleCombox = new QComboBox(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mScaleCombox, "Display", "scaleCombox", "scale combox of display");
    mScaleCombox->setObjectName("scaleCombox");

    KLabel *scaleLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(scaleLabel, "Display", "scaleCombox", "scale label of display");
    //~ contents_path /Display/Screen Zoom
    scaleLabel->setText(tr("Screen Zoom"));
    scaleLabel->setFixedWidth(108);
    scaleLayout->setSpacing(16);
    scaleLayout->addWidget(scaleLabel);
    scaleLayout->addWidget(mScaleCombox);
    KHLineFrame* scaleFrame = new KHLineFrame(m_scaleFrame);
    vbox->addWidget(scaleFrame);
    vbox->addWidget(m_scaleFrame);
    connect(mScaleCombox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [=](int index){
        slotScaleChanged(index);
    });
    initScaleItem();
    setEnableBySecurity();
    slotEnableWidget();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setEnableBySecurity()));

    scaleLabel->setAccessibleName(tr("Screen Zoom"));
    scaleLabel->setBuddy(mScaleCombox);
    mScaleCombox->setAccessibleName(tr("Screen Zoom"));
}

double OutputConfig::getScreenScale()
{
    double scale = 1.0;
    if (QGSettings::isSchemaInstalled(SCALE_SCHEMAS)) {
        if (mDpiSettings->keys().contains("scalingFactor")) {
            scale = mDpiSettings->get(SCALE_KEY).toDouble();
        }
    }
    return scale;
}

void OutputConfig::initConnection()
{
    connect(mOutput.data(), &KScreen::Output::isConnectedChanged,
            this, [=]() {
        if (!mOutput->isConnected()) {
            setVisible(false);
        }
    });
    connect(mOutput.data(), &KScreen::Output::isEnabledChanged,
            this, [=]() {
        emit enabledChanged();
    });

    connect(mOutput.data(), &KScreen::Output::rotationChanged,
            this, [=]() {
        const int index = mRotation->findData(mOutput->rotation());
        mRotation->blockSignals(true);
        mRotation->setCurrentIndex(index);
        mRotation->blockSignals(false);
    });

    connect(mOutput.data(), &KScreen::Output::scaleChanged,
            this, [=]() {
        slotUpdateScale();
    });
    //监听，否则无法处理修改分辨率/刷新率未保存
    connect(mOutput.data(), &KScreen::Output::currentModeIdChanged, this, [=]() {
        if (!mIsRestore || !mOutput->currentMode()) {
            mIsRestore = true;
            if (!mSetRestore) {
                return;
            }
        }
        mSetRestore = false;
        //分辨率改变时，触发该信号重新加载刷新率，用于修改分辨率之后但未保存
        if (mResolution->currentResolution() != mOutput->currentMode()->size()) {
            mResolution->setResolution(mOutput->currentMode()->size());
            qDebug() << Q_FUNC_INFO << "current Mode changed" << mOutput->currentMode()->size();
            initScaleItem();
            qDebug() << Q_FUNC_INFO << "current Mode changed after update scale";
            slotResolutionChanged(mOutput->currentMode()->size(), false);
        } else {
            //分辨率未修改，刷新率修改,用于修改刷新率之后但未保存
            for (int i = 0; i < mRefreshRate->count(); i++) {
                if (mRefreshRate->count() == 1 || \
                        refreshRateToText(mOutput->currentMode()->refreshRate()) == mRefreshRate->itemText(i)) {
                    mRefreshRate->blockSignals(true);
                    mRefreshRate->setCurrentIndex(i);
                    mRefreshRate->blockSignals(false);
                    break;
                }
            }
        }
    });

    connect(mOutput.data(), &KScreen::Output::isEnabledChanged,
            this, [=](){
        slotEnableWidget();
    });
}

void OutputConfig::setEnableBySecurity()
{
    // 深信服控件禁用
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value("displayEnable").toString();
    QStringList setItems = moduleSettings.split(",");

    qDebug() << Q_FUNC_INFO << moduleSettings;

    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        qDebug() << "set item Name: " << item.at(0) << item.at(1);
        if (item.at(0) == "mResolution") {
            m_resFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "mRotation") {
            mRotateFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "mRefreshRate") {
            m_freshFrame->setEnabled(item.at(1) == "true");
        }
        if (item.at(0) == "scaleFrame") {
            m_scaleFrame->setEnabled(item.at(1) == "true");
        }
    }
}

QString OutputConfig::scaleToString(double scale)
{
    return QString::number(scale * 100) + "%";
}

void OutputConfig::hideComponent()
{
    QDBusInterface ifc("com.kylin.screen.rotation",
                       "/",
                       "com.kylin.screen.rotation.interface",
                       QDBusConnection::systemBus());

    bool isShowRotation = !(QString(QLatin1String(kdk_system_get_hostCloudPlatform())) == "huawei");
    if (ifc.isValid()) {
        QDBusReply<QString> res = ifc.call("GetCurrentScreenStatus");
        isShowRotation = (isShowRotation && res.value().isEmpty());
    }
    mRotateFrame->setVisible(isShowRotation);
}

void OutputConfig::setOutput(const KScreen::OutputPtr &output)
{
    mOutput = output;
    initUi();
    hideComponent();
}

KScreen::OutputPtr OutputConfig::output() const
{
    return mOutput;
}

void OutputConfig::loadResolutionRefreshRateBlack() {
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value("displaySettings").toString();
    QStringList setItems = moduleSettings.split(",");
    m_resolutionRefreshRateBlackMap.clear();
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2 || (!item.at(0).contains("ResolutionRefreshRateBlackList"))) {
            continue;
        }
        QStringList resolutionRefreshRateBlack = item.at(1).split(";");
        foreach (QString item, resolutionRefreshRateBlack) {
            QStringList resolutionRefreshRateList = item.split("-");
            if (resolutionRefreshRateList.size() != 2) {
               continue;
            }
            QString resolution = resolutionRefreshRateList.at(0);
            QStringList refreshRateBlackList = resolutionRefreshRateList.at(1).split("#");
            if (!refreshRateBlackList.empty()) {
                qDebug() << Q_FUNC_INFO << "add blacklist" << resolution << resolutionRefreshRateList.at(1);
                m_resolutionRefreshRateBlackMap.insert(resolution, refreshRateBlackList);
            }
        }
    }
}

bool OutputConfig::hitResolutionRefreshRateBlack(QString resolution, QString refreshRate) {
    qDebug() << Q_FUNC_INFO << resolution << refreshRate;
    if (m_resolutionRefreshRateBlackMap.find(resolution) != m_resolutionRefreshRateBlackMap.end()) {
        if (m_resolutionRefreshRateBlackMap[resolution].count(refreshRate) > 0) {
            qDebug() << Q_FUNC_INFO << resolution << refreshRate << "hit blacklist";
            return true;
        }
    }
    return false;
}

//只修改刷新率时，不应该运行此函数
void OutputConfig::slotResolutionChanged(const QSize &size, bool emitFlag)
{
    // Ignore disconnected outputs
    if (!size.isValid()) {
        return;
    }
    bool mIsModeInit = false;
    QString modeID;
    KScreen::ModePtr selectMode;
    KScreen::ModePtr currentMode = mOutput->currentMode();
    QList<KScreen::ModePtr> modes;
    Q_FOREACH (const KScreen::ModePtr &mode, mOutput->modes()) {
        //初始化时,currentMode可能为空(比如刚插上屏幕)
        if (!currentMode || (currentMode && currentMode->size() == size)) {
            if (currentMode) {
                selectMode = currentMode;
            }
            mIsModeInit = true;
        }
        if (mode->size() == size) {
            if (!mIsModeInit || !currentMode) {
                selectMode = mode;
            }
            modes << mode;
        }
    }
    //非初始化，则设置选中(用户设置)的mode为该分辨率下刷新率最大的mode
    if (!mIsModeInit) {
        selectMode = findBestMode(selectMode->size());
    }
    modeID = selectMode->id();
    mRefreshRate->blockSignals(true);
    mRefreshRate->clear();
    mRefreshRate->blockSignals(false);
    QString selectRefresh;
    QMap<float, QString> refreshMap;
    QList<float> refreshList;
    for (int i = 0, total = modes.count(); i < total; ++i) {
        const KScreen::ModePtr mode = modes.at(i);
        QString comText  = refreshRateToText(mode->refreshRate());

        bool alreadyExisted = false;
        for (int j = 0; j < refreshList.count(); ++j) {
            if (comText == refreshRateToText(refreshList.at(j))) {
                alreadyExisted = true;
                break;
            }
        }

        if (alreadyExisted == false ) {   //不添加已经存在的项
            if (!hitResolutionRefreshRateBlack(Utils::sizeToString(mode->size()), comText)) {
                qDebug() << Q_FUNC_INFO << mode << mode->id();
                refreshMap.insert(mode->refreshRate(), mode->id());
                refreshList.append(mode->refreshRate());
            }
        }

        //mode是选中的mode，则设置
        if (mode == selectMode && refreshList.count() > 0) {
            selectRefresh = comText;
            qDebug() << Q_FUNC_INFO << "select refreshRate" << mOutput->name() << selectRefresh;
        }
    }
    std::sort(refreshList.begin(), refreshList.end(), std::greater<float>());
    for (int j = 0; j < refreshList.count(); ++j) {
        float refresh = refreshList.at(j);
        QString refreshStr = refreshRateToText(refresh);
        QString modeId;
        if (refreshMap.find(refresh) != refreshMap.end()) {
            modeId = refreshMap[refresh];
        } else {
            continue;
        }
        mRefreshRate->blockSignals(true);
        qDebug() << Q_FUNC_INFO << "add refresh" << refreshStr << "modeId" << modeId;
        mRefreshRate->addItem(refreshStr, modeId);
        if (!selectRefresh.isEmpty() && refreshStr == selectRefresh) {
            mRefreshRate->setCurrentText(refreshStr);
        }
        mRefreshRate->blockSignals(false);
    }
    if (mRefreshRate->count() == 0) {
        mRefreshRate->blockSignals(true);
        mRefreshRate->addItem(tr("Auto"), -1);
        mRefreshRate->blockSignals(false);
    } else {
        if (-1 == mRefreshRate->currentIndex()) {
            modeID = mRefreshRate->itemData(0).toString();
            // 避免选择50hz以下刷新率为空
            mRefreshRate->blockSignals(true);
            mRefreshRate->setCurrentIndex(0);
            mRefreshRate->blockSignals(false);
        }
    }

    mOutput->setCurrentModeId(modeID);

    if (!mIsModeInit) {
        mIsRestore = false;
        if (emitFlag) {
            changeItm = RESOLUTION;
            Q_EMIT changed();
        }
    }
}

void OutputConfig::slotRotationChanged(int index)
{
    KScreen::Output::Rotation rotation
        = static_cast<KScreen::Output::Rotation>(mRotation->itemData(index).toInt());
    mOutput->blockSignals(true);
    mOutput->setRotation(rotation);
    mOutput->blockSignals(false);
    mOutput.data()->blockSignals(true);

    changeItm = ORIENTATION;
    Q_EMIT toSetScreenPos();//要在save之前修正坐标
    Q_EMIT changed();
    UkccCommon::buriedSettings(QString("display"), QString("mRotation"), QString("select"), QString::number(index));
    //widget.cpp中收到changed信号会延迟200+1000ms,因此这里相应延迟，避免接收到信号改变导致方向多次修改。
    QTimer::singleShot(1400, this, [=](){
        mOutput.data()->blockSignals(false);
    });
}

void OutputConfig::slotScaleChanged(int index)
{
    qDebug() << Q_FUNC_INFO << "scaleCombo indexChanged index" << index << "showPrefer" << m_showPrefer << "preferIndex" << m_preferIndex << "preferScale" << m_preferScale;
    qreal kscreenScale = mScaleCombox->itemData(index).toDouble();
    if (m_showPrefer && index == m_preferIndex) {
        qDebug() << Q_FUNC_INFO << "scaleCombo indexChanged to prefer scale" << m_preferScale;
        kscreenScale = m_preferScale;
    }

    disconnect(mOutput.data(), &KScreen::Output::scaleChanged,
            this, &OutputConfig::slotUpdateScale);
    mOutput->setScale(kscreenScale);
    connect(mOutput.data(), &KScreen::Output::scaleChanged,
            this, &OutputConfig::slotUpdateScale);
    changeItm = SCALE;
    UkccCommon::buriedSettings(QString("display"), QString("U-mScaleCombox"), QString("select"), mScaleCombox->currentText());
    Q_EMIT changed();
}



void OutputConfig::rotationDbusSlot(bool autoRotation)
{
    mRotation->setEnabled(!autoRotation);
    mAutoRotation->blockSignals(true);
    mAutoRotation->setChecked(autoRotation);
    mAutoRotation->blockSignals(false);
}

void OutputConfig::tabletChanged(bool tabletMode)
{
    QDBusReply<bool> isSupportedAuto = mStatusManager->call("is_supported_autorotation");
    qDebug() << "tabletmode = " << tabletMode << "isSupportedAuto = " << isSupportedAuto;
    if (tabletMode && isSupportedAuto) {
        mAutoRotationFrame->setVisible(true);
    } else {
        mAutoRotationFrame->setVisible(false);
    }
}

void OutputConfig::rotationDirectionSlot(QString rotationDirection)
{
    mRotation->blockSignals(true);
    mRotation->setCurrentIndex(mRotation->findData(mOutput->rotation()));
    mRotation->blockSignals(false);
}

void OutputConfig::slotRefreshRateChanged(int index)
{
    QString modeId;
    modeId = mRefreshRate->itemData(index).toString();
    qDebug() << "(slotRefreshRateChanged)modeId is:" << modeId << endl;
    mOutput->blockSignals(true);
    mIsRestore = false;
    mOutput->setCurrentModeId(modeId);
    mOutput->blockSignals(false);
    changeItm = FREQUENCY;
    Q_EMIT changed();
    UkccCommon::buriedSettings(QString("display"), QString("mRefreshRate"), QString("select"), mRefreshRate->currentText());
}

void OutputConfig::slotEnableWidget()
{
    bool isEnable = mOutput.data()->isEnabled();
    mResolution->setEnabled(isEnable);
    mRotation->setEnabled(isEnable);
    mRefreshRate->setEnabled(isEnable);
    if (mScaleCombox) {
        mScaleCombox->setEnabled(isEnable);
    }
}

void OutputConfig::setShowScaleOption(bool showScaleOption)
{
    mShowScaleOption = showScaleOption;
    if (mOutput) {
        initUi();
    }
}

bool OutputConfig::showScaleOption() const
{
    return mShowScaleOption;
}

// 拿取配置
void OutputConfig::initConfig(const KScreen::ConfigPtr &config)
{
    mConfig = config;
}

void OutputConfig::initScaleItem()
{
    if (!((UkccCommon::isOpenkylin() || UkccCommon::isV11()) && UkccCommon::isWayland())) {
        qDebug() << Q_FUNC_INFO << "skip initScaleItem";
        return;
    }
    mScaleCombox->blockSignals(true);
    if (!mOutput->currentMode())
        return;
    QSize scalesize = mOutput->currentMode()->size();
    is9100VgaGpu();
    mScaleCombox->clear();
    mScaleCombox->addItem("100%", 1.0);
    if (scalesize.width() > 1024 ) {
        mScaleCombox->addItem("125%", 1.25);
    }
    if (scalesize.width() == 1920 ) {
        mScaleCombox->addItem("150%", 1.5);
    }
    if (scalesize.width() > 1920) {
        mScaleCombox->addItem("150%", 1.5);
        mScaleCombox->addItem("175%", 1.75);
    }
    if (scalesize.width() >= 2160) {
        mScaleCombox->addItem("200%", 2.0);
    }
    if (scalesize.width() > 2560 && !(m_is9100 && scalesize.width() >= 3840)) {
        mScaleCombox->addItem("225%", 2.25);
    }
    if (scalesize.width() > 3072 && !(m_is9100 && scalesize.width() >= 3840)) {
        mScaleCombox->addItem("250%", 2.5);
    }
    if (scalesize.width() > 3840 && !(m_is9100 && scalesize.width() >= 3840)) {
        mScaleCombox->addItem("275%", 2.75);
    }

    if (mScaleCombox->findData(mOutput->scale()) == -1) {
        mOutput->setScale(1);
        qDebug() << Q_FUNC_INFO << mOutput->name() << "output scale" << mOutput->scale() << "not in mScaleCombox";
;    }

    mScaleCombox->setCurrentText(QString::number(mOutput->scale() * 100) + "%");

    m_preferScale = 0;
    qDebug() << Q_FUNC_INFO << "before GetPreferredScale";
    if (m_usdDbus) {
        QSize sizeMm = mOutput->sizeMm();
        qDebug() << Q_FUNC_INFO << "GetPreferredScale scalesize" << scalesize << "sizeMm" << sizeMm << mOutput->name();
        QDBusReply<double> reply = m_usdDbus->call("GetPreferredScale", scalesize.width(), scalesize.height(), sizeMm.width(), sizeMm.height());
        double preferScale = 0.0;
        if (!reply.isValid() || (reply.error().type() == QDBusError::ErrorType::UnknownMethod)) {
            qDebug() << Q_FUNC_INFO << "GetPreferredScale is invalid";
            m_showPrefer = false;
        } else {
            preferScale = reply.value();
            m_showPrefer = true;
            qDebug() << Q_FUNC_INFO << "GetPreferredScale scale" << preferScale;
        }
        int index  = mScaleCombox->findData(preferScale);
        qDebug() << Q_FUNC_INFO << "preferScale in scaleCombox index" << index << "scaleCombox size" << mScaleCombox->count();
        if (m_showPrefer && (index >= 0 && index < mScaleCombox->count())) {
            mScaleCombox->setItemText(index, QString("%1%2").arg(QString::number(preferScale * 100) + "%").arg(tr("(Recommend)")));
            m_preferScale = preferScale;
            m_preferIndex = index;
            qDebug() << Q_FUNC_INFO << "preferScale in scaleCombox" << m_preferScale << "preferIndex" << m_preferIndex;
        } else {
            m_showPrefer = false;
            qDebug() << Q_FUNC_INFO << "preferScale not in scaleCombox" << preferScale;
        }
    }
    qDebug() << Q_FUNC_INFO << mOutput->name() << "init Scale" << mScaleCombox->currentText() << scalesize;
    mScaleCombox->blockSignals(false);
}

QString OutputConfig::refreshRateToText(float refreshRate)
{
    QRegExp rx;
    rx.setPattern("(\\.){0,1}0+$");
    return QStringLiteral("%1 Hz").arg((QString::number(refreshRate,'f',2)).replace(rx,""));
}

double OutputConfig::getGlobalData(const OutputPtr &output)
{
    QString hash = mConfig->connectedOutputsHash();

    QString scaleDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
            %QStringLiteral("/kscreen/scale/");
    QFile file(scaleDir % hash);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file" << file.fileName();
        return 0;
    }

    QByteArray readBy=file.readAll();
    QJsonParseError error;
    QJsonDocument readDoc=QJsonDocument::fromJson(readBy,&error);
    QJsonArray obj=readDoc.array();

    for (int i = 0 ; i < obj.size(); i++) {
        QJsonObject faObj= obj[i].toObject();
        if (faObj["id"].toString() == output->hashMd5()) {
            return faObj["scale"].toDouble();
        }

    }
    return 0;
}

void OutputConfig::setRestore()
{
    mSetRestore = true;
}

KScreen::Output::Rotation OutputConfig::getRotationStatus() {
    qDebug() << Q_FUNC_INFO << "output name" << mOutput->name() << "rotation" << mOutput->rotation();
    return mOutput->rotation();
}

bool OutputConfig::is9100VgaGpu()
{
    QProcess process;
    process.start("bash", QStringList() << "-c"
                  << "lspci -n -s `lspci | grep VGA | awk '{print $1}'`");
    if (!process.waitForFinished(3000)) {
        return false;
    }

    const QString output = QString::fromLocal8Bit(
        process.readAllStandardOutput() + process.readAllStandardError());

    m_is9100 = output.contains(QStringLiteral("9100")) ? true : false;
    return m_is9100;
}

KScreen::ModePtr OutputConfig::findBestMode(const QSize &size)
{
    KScreen::ModePtr m_mode;
    float refreshRate = 0;
    Q_FOREACH (const KScreen::ModePtr &mode, mOutput->modes()) {
        if (mode->size() == size && mode->refreshRate() > refreshRate) {
            refreshRate = mode->refreshRate();
            m_mode = mode;
        }
    }
    return m_mode;
}
void OutputConfig::slotUpdateScale()
{
    const int index = mScaleCombox->findData(mOutput->scale());
    mScaleCombox->blockSignals(true);
    mScaleCombox->setCurrentIndex(index);
    mScaleCombox->blockSignals(false);
}
