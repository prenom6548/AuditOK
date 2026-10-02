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
#include "unifiedoutputconfig.h"
#include "resolutionslider.h"
#include "scalesize.h"
#include "utils.h"
#include "ukcccommon.h"
using namespace ukcc;

#include <QComboBox>
#include <QIcon>
#include <QLabel>
#include <QGridLayout>
#include <QSpacerItem>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QStyledItemDelegate>
#include <QFile>
#include <QVector>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QProcess>

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/config.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/setconfigoperation.h>
#include <klabel.h>
#include <kysdk/applications/accessinfohelper.h>
#include <kysdk/applications/klineframe.h>

using namespace kdk;
using namespace KScreen;

bool operator<(const QSize &s1, const QSize &s2)
{
    return s1.width() * s1.height() < s2.width() * s2.height();
}

template<>
bool qMapLessThanKey(const QSize &s1, const QSize &s2)
{
    return s1 < s2;
}

UnifiedOutputConfig::UnifiedOutputConfig(const KScreen::ConfigPtr &config, QWidget *parent) :
    OutputConfig(parent),
    mConfig(config)
{
    mIsRestore = true;
    if (!m_usdDbus) {
        m_usdDbus = new QDBusInterface("org.ukui.SettingsDaemon",
                                       "/org/ukui/SettingsDaemon/xrandr",
                                       "org.ukui.SettingsDaemon.xrandr",
                                       QDBusConnection::sessionBus(), this);
    }
}

UnifiedOutputConfig::~UnifiedOutputConfig()
{
}

void UnifiedOutputConfig::setOutput(const KScreen::OutputPtr &output)
{
    mOutput = output;

    mClones.clear();
    mClones.reserve(mOutput->clones().count());
    Q_FOREACH (int id, mOutput->clones()) {
        mClones << mConfig->output(id);
    }
    mClones << mOutput;
    qDebug() << Q_FUNC_INFO << "UnifiedOutputConfig setOutput mClones add mOutput" << mOutput->name() << mClones.size() << mClones;

    OutputConfig::setOutput(output);
}

void UnifiedOutputConfig::initUi()
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(0);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    KScreen::OutputPtr fakeOutput = createFakeOutput();
    mResolution = new ResolutionSlider(fakeOutput, this);

    //监听，否则无法处理修改分辨率/刷新率未保存
    connect(mOutput.data(), &KScreen::Output::currentModeIdChanged,
            this, &UnifiedOutputConfig::slotRestoreResoltion);

    connect(mOutput.data(), &KScreen::Output::rotationChanged,
            this, &UnifiedOutputConfig::slotRestoreRatation);

    QLabel *resLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(resLabel, "Display", "resolutionLabel", "unify resolution label of display");
    resLabel->setText(tr("Resolution"));
    resLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    resLabel->setFixedWidth(108);

    QHBoxLayout *resLayout = new QHBoxLayout();
    resLayout->setContentsMargins(16, 0, 16, 0);
    resLayout->addWidget(resLabel);
    resLayout->setSpacing(16);
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
    KDK_EXTEND_ALL_INFO_FORMAT(resLabel, "Display", "resolutionLabel", "unify resolution label of display");

    QLabel *rotateLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(rotateLabel, "Display", "rotationLabel", "unify rotation label of display");
    rotateLabel->setText(tr("Orientation"));
    rotateLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    rotateLabel->setFixedWidth(108);

    mRotation->addItem(tr("Arrow-Up"), KScreen::Output::None);
    mRotation->addItem(tr("90° Arrow-Right"), KScreen::Output::Right);
    mRotation->addItem(tr("90° Arrow-Left"), KScreen::Output::Left);
    mRotation->addItem(tr("Arrow-Down"), KScreen::Output::Inverted);

    int index = mRotation->findData(mOutput->rotation());
    mRotation->setCurrentIndex(index);

    connect(mRotation, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &UnifiedOutputConfig::slotRotationChangedDerived);

    QHBoxLayout *roatateLayout = new QHBoxLayout();
    roatateLayout->setContentsMargins(16, 0, 16, 0);
    roatateLayout->addWidget(rotateLabel);
    roatateLayout->setSpacing(16);
    roatateLayout->addWidget(mRotation);
    mRotateFrame = new UkccFrame(this, UkccFrame::BorderRadiusStyle::None, true);
    mRotateFrame->setLayout(roatateLayout);
    KHLineFrame* rotationFrame = new KHLineFrame(mRotateFrame);
    vbox->addWidget(rotationFrame);
    vbox->addWidget(mRotateFrame);

    // 自动旋转
    mAutoRotation = new KSwitchButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mAutoRotation, "Display", "rotationLabel", "unify auto rotation button of display");

    QLabel *autoRotateLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(autoRotateLabel, "Display", "rotationLabel", "unify auto rotation label of display");
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

    // 统一输出刷新率下拉框
    mRefreshRate = new QComboBox(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mRefreshRate, "Display", "refreshRateCombox", "unify refresh rate combox of display");
    QLabel *freshLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(freshLabel, "Display", "refreshRateLabel", "unify refresh rate label of display");
    freshLabel->setText(tr("Frequency"));
    freshLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    freshLabel->setFixedWidth(108);

    QHBoxLayout *freshLayout = new QHBoxLayout();
    freshLayout->setContentsMargins(16, 0, 16, 0);
    freshLayout->setSpacing(16);
    freshLayout->addWidget(freshLabel);
    freshLayout->addWidget(mRefreshRate);

    m_freshFrame = new UkccFrame(this, UkccFrame::BorderRadiusStyle::None, true);
    m_freshFrame->setLayout(freshLayout);
    m_freshLineFrame = new KHLineFrame(this);
    vbox->addWidget(m_freshLineFrame);
    vbox->addWidget(m_freshFrame);


    slotResolutionChanged(mResolution->currentResolution(), true);
    connect(mRefreshRate, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, &UnifiedOutputConfig::slotRefreshRateChanged);
    QObject::connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished, this,
                         [&](KScreen::ConfigOperation *op) {
        KScreen::ConfigPtr sConfig = qobject_cast<KScreen::GetConfigOperation *>(op)->config();
        KScreen::OutputPtr sOutput = sConfig -> primaryOutput();

        for (int i = 0; i < mRefreshRate->count(); ++i) {
            if (!sOutput.isNull() && !sOutput->currentMode().isNull() && mRefreshRate->itemText(i) == refreshRateToText(sOutput->currentMode()->refreshRate())) {
                mRefreshRate->setCurrentIndex(i);
            }
        }
    });

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

    m_scaleFrame = new UkccFrame(this, UkccFrame::BorderRadiusStyle::None, true);

    QHBoxLayout *scaleLayout = new QHBoxLayout(m_scaleFrame);
    scaleLayout->setContentsMargins(16, 0, 16, 0);
    scaleLayout->setSpacing(16);
    mScaleCombox = new QComboBox(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mScaleCombox, "Display", "scaleCombox", "unify scale combox of display");
    mScaleCombox->setObjectName("scaleCombox");

    KLabel *scaleLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(scaleLabel, "Display", "scaleCombox", "unify scale label of display");
    //~ contents_path /Display/Screen Zoom
    scaleLabel->setText(tr("Screen Zoom"));
    scaleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    scaleLabel->setFixedWidth(108);

    scaleLayout->addWidget(scaleLabel);
    scaleLayout->addWidget(mScaleCombox);

    connect(mScaleCombox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this, [=](int index){
        slotScaleChanged(index);
    });
    KHLineFrame* scaleFrame = new KHLineFrame(m_scaleFrame);
    vbox->addWidget(scaleFrame);
    vbox->addWidget(m_scaleFrame);

    connect(mOutput.data(), &KScreen::Output::scaleChanged,
            this, &OutputConfig::slotUpdateScale);

    connect(mOutput.data(), &KScreen::Output::currentModeIdChanged,
            this, &UnifiedOutputConfig::initScaleItem);
    initScaleItem();
    setEnableBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setEnableBySecurity()));

    scaleLabel->setAccessibleName(tr("Screen Zoom"));
    scaleLabel->setBuddy(mScaleCombox);
    mScaleCombox->setAccessibleName(tr("Screen Zoom"));
}

void UnifiedOutputConfig::resolutionChangedSlot(QSize size, bool emitFlag) {
    qDebug() << Q_FUNC_INFO << "resolution changed" << size << "m_showPrefer" << m_showPrefer << "m_preferScale" << m_preferScale << mOutput->name();
    slotResolutionChanged(size, emitFlag);
    initScaleItem();
    if (m_showPrefer && m_preferIndex >= 0 && (m_preferIndex <= mScaleCombox->count())) {
        qDebug() << Q_FUNC_INFO << "resolutionChangedSlot changed scale" << mOutput->scale() << "to prefer scale" << m_preferScale << "prefer index" << m_preferIndex << mOutput->name() << mOutput->currentMode();
        mScaleCombox->blockSignals(true);
        mScaleCombox->setCurrentIndex(m_preferIndex);
        mScaleCombox->blockSignals(false);
        mOutput->blockSignals(true);
        mOutput->setScale(m_preferScale);
        mOutput->blockSignals(false);
        Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
            clone->setScale(m_preferScale);
        }
        mOutput->blockSignals(false);
    }
    UkccCommon::buriedSettings(QString("display"), QString("U-mResolution"), QString("select"), Utils::sizeToString(size));
}

void UnifiedOutputConfig::setEnableBySecurity()
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

KScreen::OutputPtr UnifiedOutputConfig::createFakeOutput()
{
    // Find set of common resolutions
    QMap<QSize, int> commonSizes;
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        QList<QSize> processedSizes;
        if (clone.isNull()) {
            qDebug() << Q_FUNC_INFO << "createFakeOutput clone output is null" << mClones.size();
            continue;
        } else {
            qDebug() << Q_FUNC_INFO << "createFakeOutput clone output is not null" << mClones.size();
        }
        Q_FOREACH (const KScreen::ModePtr &mode, clone->modes()) {
            // Make sure we don't count some modes multiple times because of different
            // refresh rates
            if (processedSizes.contains(mode->size())) {
                continue;
            }

            processedSizes << mode->size();

            if (commonSizes.keys().contains(mode->size())) {
                commonSizes[mode->size()]++;
            } else {
                commonSizes.insert(mode->size(), 1);
            }
        }
    }

    KScreen::OutputPtr fakeOutput(new KScreen::Output);

    // This will give us list of resolution that are shared by all outputs
    QList<QSize> commonResults = commonSizes.keys(mClones.count());
    // If there are no common resolution, fallback to smallest preferred mode
    if (commonResults.isEmpty()) {
        QSize smallestMode;
        Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
            if (!smallestMode.isValid() || clone->preferredMode()->size() < smallestMode) {
                smallestMode = clone->preferredMode()->size();
            }
        }
        commonResults << smallestMode;
    }
    std::sort(commonResults.begin(), commonResults.end());

    KScreen::ModeList modes;
    Q_FOREACH (const QSize &size, commonResults) {
        KScreen::ModePtr mode(new KScreen::Mode);
        mode->setSize(size);
        mode->setId(Utils::sizeToString(size));
        mode->setName(mode->id());
        modes.insert(mode->id(), mode);
    }
    fakeOutput->setModes(modes);
    if (!mOutput->currentModeId().isEmpty()) {
        fakeOutput->setCurrentModeId(Utils::sizeToString(mOutput->currentMode()->size()));
    } else {
        fakeOutput->setCurrentModeId(Utils::sizeToString(commonResults.last()));
    }

    return fakeOutput;
}

void UnifiedOutputConfig::slotResolutionChanged(const QSize &size, bool emitFlag)
{
    // Ignore disconnected outputs
    if (!size.isValid()) {
        return;
    }
    bool mIsModeInit = true;
    QVector<QString>Vrefresh;
    bool mIsCloneMode = isCloneMode();
    mRefreshRate->blockSignals(true);
    mRefreshRate->clear();
    mRefreshRate->blockSignals(false);
    QList<float> refreshList;
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        const QString &id = findBestMode(clone, size);
        if (id.isEmpty()) {
            // FIXME: Error?
            return;
        }
        //本来就是镜像模式且当前分辨率就是选中分辨率，就不需要重新设置显示参数
        //用于镜像模式下刚打开控制面板时的显示，否则显示的不是实际刷新率而是findBestMode
        if (!mIsCloneMode || size != clone->currentMode()->size()) {
            mIsModeInit = false;
            clone->blockSignals(true); //必须加blockSignals，否则在这里就会触发currentModeIdChanged的信号
            mIsRestore = false; //修改配置会触发currentModeIdChanged时，不运行slotRestoreResoltion
            clone->setCurrentModeId(id);
            clone->setPos(QPoint(0, 0));
            clone->blockSignals(false);
        }

        QList<KScreen::ModePtr> modes;
        Q_FOREACH (const KScreen::ModePtr &mode, clone->modes()) {
            if (mode->size() == size) {
                modes << mode;
            }
        }

        QVector<QString>VrefreshTemp;
        for (int i = 0, total = modes.count(); i < total; ++i) {
           const KScreen::ModePtr mode = modes.at(i);

           bool alreadyExisted = false; //判断该显示器的刷新率是否有重复的，确保同一刷新率在一个屏幕上只出现一次
           for (int j = 0; j < VrefreshTemp.size(); ++j) {
               if (refreshRateToText(mode->refreshRate()) == VrefreshTemp[j]) {
                   alreadyExisted = true;
                   break;
               }
           }
           if (alreadyExisted == false) {   //不添加重复的项
               VrefreshTemp.append(refreshRateToText(mode->refreshRate()));
               refreshList.append(mode->refreshRate());
           }
        }

        for (int i = 0; i < VrefreshTemp.size(); ++i) {
            Vrefresh.append(VrefreshTemp[i]);
        }
    }
    QSet<float> commonFreshSet;
    for (int i = 0; i < Vrefresh.size(); ++i) {
        if (Vrefresh.count(Vrefresh[i]) == mClones.size()) { //该刷新率出现次数等于屏幕数，即每个屏幕都有该刷新率
            for (int j = 0; j < refreshList.count(); ++j) {
                if (Vrefresh[i] == refreshRateToText(refreshList.at(j))) {
                    commonFreshSet.insert(refreshList.at(j));
                    qDebug() << Q_FUNC_INFO << "add fresh" << refreshList.at(j);
                    break;
                }
            }
        }
    }
    QList<float> commonFreshList = commonFreshSet.values();
    std::sort(commonFreshList.begin(), commonFreshList.end(), std::greater<float>());
    for (int j = 0; j < commonFreshList.count(); ++j) {
        float refresh = commonFreshList.at(j);
        QString refreshStr = refreshRateToText(refresh);
        mRefreshRate->blockSignals(true);
        qDebug() << Q_FUNC_INFO << "add refresh" << refreshStr;
        mRefreshRate->addItem(refreshStr);
        mRefreshRate->blockSignals(false);
    }
    // 按 2503/华为要求：主屏当前刷新率不在共同列表时，隐藏刷新率选项
    bool isHideRefresh = true;
    if (mRefreshRate->count() >= 1 && !mClones.isEmpty() && mClones[0] && mClones[0]->currentMode()) {
        float currentRereshRate = mClones[0]->currentMode()->refreshRate();
        for (int i = 0; i < mRefreshRate->count(); i++) {
            if (refreshRateToText(currentRereshRate) == mRefreshRate->itemText(i)) {
                mRefreshRate->blockSignals(true);
                mRefreshRate->setCurrentIndex(i);
                mRefreshRate->blockSignals(false);
                isHideRefresh = false;
                qDebug() << Q_FUNC_INFO << "mRefreshRate item" << mRefreshRate->itemText(i) << "currentRereshRate" << refreshRateToText(currentRereshRate) << currentRereshRate;
                break;
            }
        }
    }
    qDebug() << Q_FUNC_INFO << "m_freshFrame isHideRefresh" << isHideRefresh;
    if (m_freshLineFrame) {
        m_freshLineFrame->setHidden(isHideRefresh);
    }
    if (m_freshFrame) {
        m_freshFrame->setHidden(isHideRefresh);
    }
    if (mRefreshRate->count() == 0) {
        mRefreshRate->blockSignals(true);
        mRefreshRate->addItem(tr("Auto"), -1);
        mRefreshRate->blockSignals(false);
    }
    if (emitFlag && !mIsModeInit){
        changeItm = RESOLUTION;
        Q_EMIT changed();
    }
}

void UnifiedOutputConfig::slotRefreshRateChanged(int index)
{
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        Q_FOREACH (const KScreen::ModePtr &mode, clone->modes()) {
            if (mode->size() == mResolution->currentResolution() && \
                    refreshRateToText(mode->refreshRate()) == mRefreshRate->itemText(index)) {
                mIsRestore = false;
                clone->blockSignals(true);
                clone->setCurrentModeId(mode->id());
                clone->blockSignals(false);
            }
        }
    }
    changeItm = FREQUENCY;
    Q_EMIT changed();
    UkccCommon::buriedSettings(QString("display"), QString("U-mRefreshRate"), QString("select"), mRefreshRate->currentText());
}

void UnifiedOutputConfig::slotScaleChanged(int index)
{
    mScaleCombox->blockSignals(true);
    qDebug() << Q_FUNC_INFO << "scaleCombo indexChanged index" << index << "showPrefer" << m_showPrefer << "preferIndex" << m_preferIndex << "preferScale" << m_preferScale;
    qreal kscreenScale = mScaleCombox->itemData(index).toDouble();
    if (m_showPrefer && index == m_preferIndex) {
        qDebug() << Q_FUNC_INFO << "scaleCombo indexChanged to prefer scale" << m_preferScale;
        kscreenScale = m_preferScale;
    }
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        clone->setScale(kscreenScale);
    }
    UkccCommon::buriedSettings(QString("display"), QString("U-mScaleCombox"), QString("select"), mScaleCombox->currentText());
    changeItm = SCALE;
    Q_EMIT changed();
    mScaleCombox->blockSignals(false);
}

QString UnifiedOutputConfig::findBestMode(const KScreen::OutputPtr &output, const QSize &size)
{
    float refreshRate = 0;
    QString id;
    Q_FOREACH (const KScreen::ModePtr &mode, output->modes()) {
        if (mode->size() == size && mode->refreshRate() > refreshRate) {
            refreshRate = mode->refreshRate();
            id = mode->id();
        }
    }
    return id;
}

// 统一输出方向信号改变
void UnifiedOutputConfig::slotRotationChangedDerived(int index)
{
    KScreen::Output::Rotation rotation = static_cast<KScreen::Output::Rotation>(mRotation->itemData(index).toInt());
    Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
        qDebug() << Q_FUNC_INFO << "slotRotationChangedDerived clone name" << clone->name() << clone->isConnected() << clone->isEnabled() << mClones.size();
        if (clone->isConnected() && clone->isEnabled()) {
            clone->blockSignals(true);
            clone->setRotation(rotation);
            clone->setPos(QPoint(0, 0));
            clone->blockSignals(false);
        }
    }
    changeItm = ORIENTATION;
    Q_EMIT changed();
    UkccCommon::buriedSettings(QString("display"), QString("U-mRotation"), QString("select"), QString::number(index));
}

void UnifiedOutputConfig::rotationDbusSlot(bool autoRotation)
{
    mRotation->setEnabled(!autoRotation);
    mAutoRotation->blockSignals(true);
    mAutoRotation->setChecked(autoRotation);
    mAutoRotation->blockSignals(false);
}

void UnifiedOutputConfig::tabletChanged(bool tabletMode)
{
    QDBusReply<bool> isSupportedAuto = mStatusManager->call("is_supported_autorotation");
    qDebug() << "tabletmode = " << tabletMode << "isSupportedAuto = " << isSupportedAuto;
    if (tabletMode && isSupportedAuto) {
        mAutoRotationFrame->setVisible(true);
    } else {
        mAutoRotationFrame->setVisible(false);
    }
}

void UnifiedOutputConfig::rotationDirectionSlot(QString rotationDirection)
{
    mRotation->blockSignals(true);
    if (rotationDirection.compare("normal") == 0) {
        mRotation->setCurrentIndex(KScreen::Output::None);
    } else if (rotationDirection.compare("upside-down") == 0) {
        mRotation->setCurrentIndex(KScreen::Output::Inverted);
    } else if (rotationDirection.compare("left") == 0) {
        mRotation->setCurrentIndex(KScreen::Output::Left);
    } else if (rotationDirection.compare("right") == 0) {
        mRotation->setCurrentIndex(KScreen::Output::Right);
    }
    mRotation->blockSignals(false);
}

void UnifiedOutputConfig::slotRestoreResoltion()
{
    if (!mIsRestore || !mOutput->currentMode()) {
        mIsRestore = true;
        if (!mSetRestore) {
            return;
        }
    }
    mSetRestore = false;
    if (mResolution->currentResolution() != mOutput->currentMode()->size()) { //分辨率改变时，触发该信号重新加载刷新率，用于修改分辨率之后但未保存
        mResolution->setResolution(mOutput->currentMode()->size()); //这里面不会触发分辨率改变信号
        Q_EMIT scaleChanged(mOutput->currentMode()->size());
        slotResolutionChanged(mOutput->currentMode()->size(), false);
    } else { //分辨率未修改，刷新率修改,用于修改刷新率之后但未保存
        for (int i = 0; i < mRefreshRate->count(); i++) {
           if (refreshRateToText(mOutput->currentMode()->refreshRate()) == mRefreshRate->itemText(i)\
                   || mRefreshRate->count() == 1) {
               mRefreshRate->blockSignals(true);
               mRefreshRate->setCurrentIndex(i);
               mRefreshRate->blockSignals(false);
               break;
           }
        }
    }
}

void UnifiedOutputConfig::slotRestoreRatation()
{
    mRotation->blockSignals(true);
    mRotation->setCurrentIndex(mRotation->findData(mOutput->rotation()));
    mRotation->blockSignals(false);
}

bool UnifiedOutputConfig::isCloneMode()
{
    /*
     *不能直接用isVisible判断是否为镜像模式
     *设置镜像模式时，visiable总是true，但此时还未设置currentMode
     *导致某些情况异常
     */
    //return this->isVisible();    //显示则表示是统一输出
    if (!mClones.isEmpty() && mClones[0] && mClones[0]->currentMode()) {
        QSize cloneSize(mClones[0]->currentMode()->size());
        QPoint clonePos(mClones[0]->pos());
        Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
            if (clone->currentMode() && (clone->currentMode()->size() != cloneSize || clone->pos() != clonePos)) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }

}

void UnifiedOutputConfig::initScaleItem()
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
        qDebug() << Q_FUNC_INFO << mOutput->name() << "output scale" << mOutput->scale() << "not in mScaleCombox";
        Q_FOREACH (const KScreen::OutputPtr &clone, mClones) {
            clone->setScale(1);
        }
    }

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

double UnifiedOutputConfig::getGlobalData(OutputPtr output)
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
        if (faObj["id"].toString() == output->hashMd5())
            return faObj["scale"].toDouble();
    }
    return 0;
}

QFrame *UnifiedOutputConfig::setLine(QFrame *frame)
{
    QFrame *line = new QFrame(frame);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}
