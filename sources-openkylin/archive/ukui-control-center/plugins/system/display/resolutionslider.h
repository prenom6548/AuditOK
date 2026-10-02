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
#ifndef RESOLUTIONSLIDER_H
#define RESOLUTIONSLIDER_H

#include <QWidget>
#include <QSet>

#include <KF5/KScreen/kscreen/output.h>

class QSlider;
class QLabel;
class QComboBox;
class QStyledItemDelegate;

class ResolutionSlider : public QWidget
{
    Q_OBJECT

public:
    explicit ResolutionSlider(const KScreen::OutputPtr &output, QWidget *parent = nullptr);
    ~ResolutionSlider() override;

    QSize currentResolution() const;
    QSize getMaxResolution() const;

    void setResolution(const QSize &size);
    // 焦点在内部 QComboBox；buddy 须绑到 combo，否则读屏读不到「分辨率」
    QComboBox *comboBox() const;
    void setBuddyLabel(QLabel *label);

Q_SIGNALS:
    void resolutionChanged(const QSize &size, bool emitFlag = true);

public Q_SLOTS:
    void slotValueChanged(int);

private:
    void init();
    void removeSolution();
    void applyBuddyLabel();

private:
    KScreen::OutputPtr mOutput;

    QList<QSize> mModes;
    QList<QSize> mExcludeModes;

    QComboBox *mComboBox = nullptr;
    QLabel *mBuddyLabel = nullptr;

    bool mIsWayland = false;
};

#endif // RESOLUTIONSLIDER_H
