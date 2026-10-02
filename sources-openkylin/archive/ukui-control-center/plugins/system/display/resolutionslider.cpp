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
#include "resolutionslider.h"
#include "utils.h"
#include "scalesize.h"

#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QFile>
#include <QStyledItemDelegate>
#include <QDBusInterface>

#include <KF5/KScreen/kscreen/output.h>

#include <kysdk/kysdk-system/libkysysinfo.h>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

static bool sizeLessThan(const QSize &sizeA, const QSize &sizeB)
{
    return sizeA.width() * sizeA.height() < sizeB.width() * sizeB.height();
}

ResolutionSlider::ResolutionSlider(const KScreen::OutputPtr &output, QWidget *parent) :
    QWidget(parent),
    mOutput(output)
{
    QString sessionType = getenv("XDG_SESSION_TYPE");
    if (sessionType.compare("wayland", Qt::CaseSensitive)) {
       mExcludeModes.push_back(QSize(1152, 864));
    }

    connect(output.data(), &KScreen::Output::modesChanged,
            this, &ResolutionSlider::init);
    init();
}

ResolutionSlider::~ResolutionSlider()
{
}

void ResolutionSlider::init()
{
    mModes.clear();
    Q_FOREACH (const KScreen::ModePtr &mode, mOutput->modes()) {
        if (mModes.contains(mode->size())
            || (mode->size().width() * mode->size().height()) < (1024 * 768)
            || mExcludeModes.contains(mode->size())) {
            continue;
        }
        mModes << mode->size();
    }
    std::sort(mModes.begin(), mModes.end(), sizeLessThan);

    delete layout();
    delete mComboBox;
    mComboBox = nullptr;

    mComboBox = new QComboBox(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mComboBox, "Display", "resolutionCombox", "resolution combox of display");
    mComboBox->setAccessibleName(tr("Resolution"));
    applyBuddyLabel();
    QGridLayout *layout = new QGridLayout(this);
    // Avoid double margins
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mComboBox, 0, 0, 1, 1);
    if (!mModes.empty()) {
        std::reverse(mModes.begin(), mModes.end());

        int currentModeIndex = -1;
        int preferredModeIndex = -1;
        Q_FOREACH (const QSize &size, mModes) {
            mComboBox->addItem(Utils::sizeToString(size));

            if (mOutput->currentMode() && (mOutput->currentMode()->size() == size)) {
                currentModeIndex = mComboBox->count() - 1;
            } else if (mOutput->preferredMode() && (mOutput->preferredMode()->size() == size)) {
                preferredModeIndex = mComboBox->count() - 1;
            }
        }
        if (currentModeIndex != -1) {
            mComboBox->setCurrentIndex(currentModeIndex);
        } else if (preferredModeIndex != -1) {
            mComboBox->setCurrentIndex(preferredModeIndex);
        }

        connect(mComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &ResolutionSlider::slotValueChanged, Qt::UniqueConnection);
        Q_EMIT resolutionChanged(mModes.at(mComboBox->currentIndex()), false);
    }
}

void ResolutionSlider::removeSolution()
{
    if (mComboBox) {
        QString plat = QString(QLatin1String(kdk_system_get_hostVirtType()));
        if (plat == "qemu" || plat == "kvm") {
            for (int i = 0; i < mComboBox->count(); i++) {
                int index = mComboBox->currentIndex();
                if (index != i) {
                    mComboBox->removeItem(i);
                    i = 0;
                }
            }
        }
    }
}

QSize ResolutionSlider::currentResolution() const
{
    if (mModes.isEmpty()) {
        return QSize();
    }

    if (mModes.size() < 2) {
        return mModes.first();
    }

    const int i = mComboBox->currentIndex();
    return i > -1 ? mModes.at(i) : QSize();
}

QSize ResolutionSlider::getMaxResolution() const
{
    if (mModes.isEmpty()) {
        return QSize();
    }
    return mModes.first();
}

void ResolutionSlider::setResolution(const QSize &size)
{
    if (mComboBox) {
        mComboBox->blockSignals(true);
        mComboBox->setCurrentIndex(mModes.indexOf(size));
        mComboBox->blockSignals(false);
    }
}

QComboBox *ResolutionSlider::comboBox() const
{
    return mComboBox;
}

void ResolutionSlider::setBuddyLabel(QLabel *label)
{
    mBuddyLabel = label;
    applyBuddyLabel();
}

void ResolutionSlider::applyBuddyLabel()
{
    // init 会销毁重建 combo，需重新 setBuddy，否则 AT-SPI 无 LABELLED_BY
    if (mBuddyLabel && mComboBox) {
        mBuddyLabel->setBuddy(mComboBox);
    }
}

void ResolutionSlider::slotValueChanged(int value)
{
    const QSize &size = mModes.at(value);
    Q_EMIT resolutionChanged(size);
}
