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

#ifndef OTHERPREVIEWPAGE_H
#define OTHERPREVIEWPAGE_H

#include <QLabel>
#include <QFormLayout>
#include <QIcon>

#include "previewpage/base.h"
#include "components/icon-button.h"

class OtherPreviewPage : public BasePreviewPage
{
    Q_OBJECT
public:
    explicit OtherPreviewPage(QWidget *parent = nullptr);
    ~OtherPreviewPage() = default;

private:
    IconButton *m_icon;
    QFormLayout *m_form;
    QLabel *m_display_name_label;
    QLabel *m_type_label;
    QLabel *m_time_modified_label;
    QLabel *m_time_access_label;

    // BasePreviewPage interface
public:
    void cancel() override;

public Q_SLOTS:
    void updateInfo(PreviewHubFileInfo *info) override;

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent* size) override;
};

#endif // OTHERPREVIEWPAGE_H
