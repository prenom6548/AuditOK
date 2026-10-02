/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "wayland-screen-manager.h"
#include "../xrandr/xrandr-config.h"
#include "../xrandr/xrandr-output.h"
#include "calibrate-touch-device.h"
WaylandScreenManager::WaylandScreenManager()
{

}

WaylandScreenManager::~WaylandScreenManager()
{

}


void WaylandScreenManager::setScreenConfig()
{
    connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished,
            this, [this](KScreen::ConfigOperation* op) {
        if (op->hasError()) {
            SYS_LOG(LOG_DEBUG,"Error getting initial configuration：%s",op->errorString().toLatin1().data());
            UsdBaseClass::printTimeWithMsg("exit..",__FUNCTION__, __LINE__);
            std::exit(0);
        }
        m_outputsConfig = std::unique_ptr<xrandrConfig>(new xrandrConfig(qobject_cast<KScreen::GetConfigOperation*>(op)->config()));
        m_outputsConfig->setValidityFlags(KScreen::Config::ValidityFlag::RequireAtLeastOneEnabledScreen);
        SYS_LOG(LOG_DEBUG,"get.SUCCESS");
        if (m_outputsConfig->fileExists()) {
            SYS_LOG(LOG_DEBUG,"set with config");
            m_outputsConfig= m_outputsConfig->readFile(false);
            applyConfig();
        } else {
            SYS_LOG(LOG_DEBUG,"set without config");
            setOutputsModeToCloneWithPreferredMode();
        }
    });
#
}

void WaylandScreenManager::setOutputsModeToCloneWithPreferredMode()
{
    bool hadFindFirstScreen = false;

    QList<QSize> slist;
    slist.clear();
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (false == output->isConnected()) {
            continue;
        }

        output->setEnabled(true);
        output->setPos(QPoint(0,0));
        output->setRotation(static_cast<KScreen::Output::Rotation>(1));
        if (false == hadFindFirstScreen) {
            hadFindFirstScreen = true;
            Q_FOREACH(auto primaryMode, output->modes()) {
                if (!slist.contains(primaryMode->size())) {
                    slist.append(primaryMode->size());
                }
            }
            continue;
        }

        QList<QSize> templist = slist;
        for (auto size: slist) {
            bool findSameSize = false;
            USD_LOG(LOG_DEBUG,"check %dx%d over",size.width(), size.height());
            Q_FOREACH(auto minjorScreenMode, output->modes()) {
                if (minjorScreenMode->size() == size) {
                    findSameSize = true;
                }
            }

            if (false == findSameSize) {
                templist.removeAll(size);
                USD_LOG(LOG_DEBUG,"remove %dx%d over",size.width(), size.height());
            }
        }
        slist = templist;

    }

    if(slist.count() > 0) {//镜像
        qSort(slist.begin(), slist.end(),[](const QSize &a, const QSize &b){
            if (a.width() == b.width())
                return a.height() > b.height();
            return a.width() > b.width();
        });//降序
    }
    double minScale = 999;
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (false == output->isConnected()) {
            continue;
        }
        if (output->preferredMode()->size() == slist[0]) {
            output->setCurrentModeId(output->preferredMode()->id());
            if (UsdBaseClass::isWayland()) {
                output->setScale(getPreferredScale(output));
                minScale = minScale > output->scale() ? output->scale() : minScale;
            }
            continue;
        }

        Q_FOREACH(auto minjorScreenMode, output->modes()) {
            if (minjorScreenMode->size() == slist[0]) {
                output->setCurrentModeId(minjorScreenMode->id());
                if (UsdBaseClass::isWayland()) {
                    output->setScale(UsdBaseClass::getPreferredScale(output->sizeMm().width(), output->sizeMm().height(),
                                                                      minjorScreenMode->size().width(), minjorScreenMode->size().height()));
                    minScale = minScale > output->scale() ? output->scale() : minScale;
                }
                break;
            }
        }
    }

    if (UsdBaseClass::isWayland()) {
        Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
            if (false == output->isConnected()) {
                continue;
            }
            output->setScale(minScale);
        }
    }
    applyConfig();
}


double WaylandScreenManager::getPreferredScale(const KScreen::OutputPtr  output)
{//只是针对于外接显示器基于当前缩放率进行计算，与设备相关的缩放放在save-param内,显示器的推荐缩放率在其他位置。

    int width = output->sizeMm().width();
    int height = output->sizeMm().height();

    return UsdBaseClass::getPreferredScale(width, height,
                                           output->preferredMode()->size().width(), output->preferredMode()->size().height());
}


void WaylandScreenManager::applyConfig()
{
    UsdBaseClass::printTimeWithMsg("ready applyConfig..",__FUNCTION__, __LINE__);
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        SYS_LOG_SHOW_OUTPUT(output);
    }
    if (m_outputsConfig->canBeApplied()) {
        connect(new KScreen::SetConfigOperation(m_outputsConfig->data()),
                &KScreen::SetConfigOperation::finished,
                this, [this]() {
            USD_LOG(LOG_DEBUG,"apply success.");
            CalibrateTouchDevice calibrateDevice;
            calibrateDevice.calibration();
            UsdBaseClass::printTimeWithMsg("set success exit..",__FUNCTION__, __LINE__);
            std::exit(0);
        });
    } else {
        SYS_LOG(LOG_ERR,"--|can't be apply|--");
        Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
            SYS_LOG_SHOW_OUTPUT(output);
        }
        std::exit(0);
    }
}
