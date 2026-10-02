/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#ifndef GRUBTHEMEWIDGET_H
#define GRUBTHEMEWIDGET_H

#include "grubthemefeature.h"
#include "historywidget.h"
#include "basewidget.h"

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QCheckBox>

class GrubThemeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GrubThemeWidget(QWidget *parent = nullptr);
    void initPreviewWidget();
    void initRightWidget();
    void addspaceritem();
    void initEditWidget();
    void initBuildWidget();
    static bool g_themeChange;
    void eidtInitWidget(const HistoryInfo &InfoData);
    void refresh();
    void useMaximumScale();
    void useRestoreScale();
    void setBuildBtnText(bool isglobal);
signals:
    void newGrubFilePath(const QString& path);
    void build();
private:
    const QString resourceGrubPath = ":/resource/background/grub-background.png";
    BasePreviewWidget *m_previewwidget;
    GrubImageWidget *m_preview;
    QWidget *m_rightwidget;
    QWidget *m_grubwidget;
    QWidget *m_buildwidget;
    QVBoxLayout *m_previewlayout;
    GrubEditButton *m_showBtn;
    QPushButton *m_buildButton;
};

#endif // GRUBTHEMEWIDGET_H
