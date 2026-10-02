/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Lu Hongxu <lhongxu@outlook.com>
 *
 */

#ifndef AUDIOPREVIEWPAGE_H
#define AUDIOPREVIEWPAGE_H

#include <QMediaPlayer>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QFormLayout>

#include "previewpage/base.h"
#include "components/slider.h"
#include "components/icon-button.h"

class AudioPreviewPage : public BasePreviewPage
{
    Q_OBJECT
public:
    explicit AudioPreviewPage(QWidget *parent = nullptr);
    ~AudioPreviewPage() = default;

private:
    QPushButton *m_button;
    QPushButton *volume_button;
    Slider *m_progress;
    QTimer *timer;
    QMediaPlayer *m_player;

    QIcon m_base_icon;
    IconButton *m_icon;
    QFormLayout *m_form;
    QLabel *m_title_label;
    QLabel *m_author_label;
//    QLabel *m_time_modified_label;
//    QLabel *m_time_access_label;

    void updateMeta();

    // BasePreviewPage interface
public Q_SLOTS:
    void updateInfo(PreviewHubFileInfo *info) override;
public:
    void cancel() override;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent* size) override;
};

#endif // AUDIOPREVIEWPAGE_H
