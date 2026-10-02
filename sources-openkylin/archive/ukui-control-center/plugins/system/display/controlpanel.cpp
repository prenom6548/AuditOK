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
#include "controlpanel.h"
#include "outputconfig.h"
#include "unifiedoutputconfig.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QDBusInterface>
#include <KF5/KScreen/kscreen/config.h>

QSize mScaleSize = QSize();

ControlPanel::ControlPanel(QWidget *parent)
    : UkccFrame(parent),
    mUnifiedOutputCfg(nullptr)
{
    this->setContainer(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mLayout = new QVBoxLayout(this);
    mLayout->setMargin(0);

    isWayland();
}

ControlPanel::~ControlPanel()
{
}

void ControlPanel::setConfig(const KScreen::ConfigPtr &config)
{
    qDeleteAll(mOutputConfigs);
    mOutputConfigs.clear();
    delete mUnifiedOutputCfg;
    mUnifiedOutputCfg = nullptr;
    mCurrentOutput.clear();

    if (mConfig) {
        mConfig->disconnect(this);
    }

    mConfig = config;
    connect(mConfig.data(), &KScreen::Config::outputAdded,
            this, [=](const KScreen::OutputPtr &output) {
        addOutputSlot(output);
    });
    connect(mConfig.data(), &KScreen::Config::outputRemoved,
            this, &ControlPanel::removeOutput);

    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        addOutput(output, false);
    }
}

void ControlPanel::addOutputSlot(const KScreen::OutputPtr &output) {
    qDebug() << Q_FUNC_INFO << "wait 100ms for addOutput";
    QTimer::singleShot(100, this, [=]() {
        addOutput(output, false);
    });
}

void ControlPanel::addOutput(const KScreen::OutputPtr &output, bool connectChanged)
{
    if (!connectChanged) {
        connect(output.data(), &KScreen::Output::isConnectedChanged,
                    this, &ControlPanel::slotOutputConnectedChanged);
    }

    if (!output->isConnected() || (output->size() == QSize(-1, -1))) return;

    OutputConfig *outputCfg = new OutputConfig(this);
    outputCfg->setVisible(false);
    outputCfg->setShowScaleOption(mConfig->supportedFeatures().testFlag(KScreen::Config::Feature::PerOutputScaling));

    outputCfg->setOutput(output);
    connect(outputCfg, &OutputConfig::changed,
            this, &ControlPanel::changed);

    connect(outputCfg, &OutputConfig::scaleChanged,
            this, &ControlPanel::scaleChanged);

    connect(outputCfg, &OutputConfig::enabledChanged,
            this, &ControlPanel::enabledChanged);

    connect(outputCfg, &OutputConfig::toSetScreenPos,
            this, [=](){
        Q_EMIT this->toSetScreenPos(output);
    });

    mLayout->addWidget(outputCfg);

    mOutputConfigs << outputCfg;

    // Wayland：激活当前屏；首次 add 时 mCurrentOutput 仍空，用刚加入的 output，避免空指针闪退
    if (mIsWayland) {
        activateOutput(mCurrentOutput.isNull() ? output : mCurrentOutput);
    }
}

void ControlPanel::setCloneMode(bool cloneMode) {
    qDebug() << Q_FUNC_INFO << "set cloneMode" << cloneMode;
    m_cloneMode = cloneMode;
}

void ControlPanel::removeOutput(int outputId)
{
    qDebug() << Q_FUNC_INFO << "m_cloneMode" << m_cloneMode << mUnifiedOutputCfg;
    if (mUnifiedOutputCfg && !m_cloneMode) {
        mUnifiedOutputCfg->deleteLater(); //避免内存泄露，插拔时会一直new，所以这里delete掉
        mUnifiedOutputCfg = nullptr;
        qDebug() << Q_FUNC_INFO << "delete mUnifiedOutputCfg" << m_cloneMode;
    }
    for (OutputConfig *outputCfg : mOutputConfigs) {
        if (!outputCfg || !outputCfg->output()) {
            continue;
        }
        if (outputCfg->output()->id() == outputId) {
            if (outputCfg->output()->isConnected() && outputCfg->output()->isEnabled()) {
                qDebug() << Q_FUNC_INFO << "removeOutput outputId" << outputId << "outputCfg output connected and isEnabled, skip remove";
                continue;
            }
            mOutputConfigs.removeOne(outputCfg);
            outputCfg->deleteLater();
            outputCfg = nullptr;
        } else {
            if (outputCfg->output()->isConnected() && !m_cloneMode) {
                outputCfg->setVisible(true);
                qDebug() << Q_FUNC_INFO << "removeOutput set outputCfg visible" << outputCfg->output()->name();
            } else {
                outputCfg->setVisible(false);
                qDebug() << Q_FUNC_INFO << "removeOutput set outputCfg inVisible" << outputCfg->output()->name();
            }
        }
    }
    if (m_cloneMode) {
        m_cloneMode = false;
    }
}

void ControlPanel::activateOutput(const KScreen::OutputPtr &output)
{
    // Ignore activateOutput when in unified mode
    //避免镜像下拔掉所有屏幕再插上导致不显示显示器内容
    if (mUnifiedOutputCfg && mUnifiedOutputCfg->isVisible()) {
        qDebug() << Q_FUNC_INFO << "activateOutput UnifiedOutputCfg skip";
        return;
    }
    if (output.isNull()) {
        qWarning() << Q_FUNC_INFO << "activateOutput skip null output";
        return;
    }

    mCurrentOutput = output;

    Q_FOREACH (OutputConfig *cfg, mOutputConfigs) {
        if (!cfg || cfg->output().isNull()) {
            continue;
        }
        cfg->setVisible(cfg->output()->id() == output->id());
        qDebug() << Q_FUNC_INFO << "activateOutput set outputCfg visible" << cfg->isVisible() << cfg->output()->name() << "output outputId" << cfg->output()->id() << "active outputId" << output->id();
    }
}

void ControlPanel::activateOutputNoParam()
{
    // Ignore activateOutput when in unified mode
    if (mUnifiedOutputCfg) {
        return;
    }

    Q_FOREACH (OutputConfig *cfg, mOutputConfigs) {
        cfg->setVisible(cfg->output()->id() == 66);
    }
}

void ControlPanel::changescalemax(const KScreen::OutputPtr &output)
{
    QSize sizescale = QSize();
    Q_FOREACH (const KScreen::ModePtr &mode, output->modes()) {
        if (sizescale.width() <= mode->size().width()) {
            sizescale = mode->size();
        }
    }
    if (mScaleSize == QSize() || mScaleSize.width() > sizescale.width()) {
        mScaleSize = sizescale;
    }
}

void ControlPanel::isWayland()
{
    QString sessionType = getenv("XDG_SESSION_TYPE");

    if (!sessionType.compare(kSession, Qt::CaseSensitive)) {
        mIsWayland = true;
    } else {
        mIsWayland = false;
    }
}

void ControlPanel::setUnifiedOutput(const KScreen::OutputPtr &output)
{
    if (output.isNull()) {
        mUnifiedOutputCfg->deleteLater();
        mUnifiedOutputCfg = nullptr;
        qDebug() << Q_FUNC_INFO << "UnifiedOutputConfig is null";
    } else {
        if (mUnifiedOutputCfg) {
            mUnifiedOutputCfg->deleteLater();
            mUnifiedOutputCfg = nullptr;
        }
        mUnifiedOutputCfg = new UnifiedOutputConfig(mConfig, this);
        qDebug() << Q_FUNC_INFO << "UnifiedOutputConfig set output start" << output->name() << output->clones().size();
        mUnifiedOutputCfg->setOutput(output);
        mUnifiedOutputCfg->setVisible(true);
        qDebug() << Q_FUNC_INFO << "UnifiedOutputConfig set output" << output->name() << mUnifiedOutputCfg->isVisible() << mLayout->count();
        mLayout->insertWidget(mLayout->count() - 2, mUnifiedOutputCfg);
        connect(mUnifiedOutputCfg, &UnifiedOutputConfig::changed,
                this, &ControlPanel::changed);
        connect(mUnifiedOutputCfg, &UnifiedOutputConfig::scaleChanged,
                this, &ControlPanel::scaleChanged);
    }
    Q_FOREACH (OutputConfig *config, mOutputConfigs) {
        // 隐藏下面控制
        config->setVisible(false);
    }
}

void ControlPanel::slotOutputConnectedChanged()
{
    const KScreen::OutputPtr output(qobject_cast<KScreen::Output *>(sender()), [](void *){
    });

    if (output->isConnected()) {
        addOutput(output, true);
    } else {
        removeOutput(output->id());
    }
}

KScreen::Output::Rotation ControlPanel::getRotationStatus() {
    for (OutputConfig *outputCfg : mOutputConfigs) {
        if (!outputCfg || !outputCfg->output()) {
            continue;
        }
        if (outputCfg->isVisible()) {
            outputCfg->setRestore();
            return outputCfg->getRotationStatus();
        }
    }
    return KScreen::Output::None;
}

void ControlPanel::setRestore()
{
    for (OutputConfig *outputCfg : mOutputConfigs) {
        if (!outputCfg || !outputCfg->output()) {
            continue;
        }
        if (outputCfg->isVisible()) {
            outputCfg->setRestore();
        }
    }
    if (mUnifiedOutputCfg && mUnifiedOutputCfg->isVisible()) {
        mUnifiedOutputCfg->setRestore();
    }
}
