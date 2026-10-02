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
#ifndef PLYMOUTHMOUTHTHEMEWIDGET_H
#define PLYMOUTHMOUTHTHEMEWIDGET_H
#include "plymouththemefeature.h"
#include "iconwidgetfeature.h"
#include "historywidget.h"
#include "basewidget.h"
#include "../fileProcess/filecheck.h"

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QButtonGroup>

class PlymouthThemeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlymouthThemeWidget(QWidget *parent = nullptr);
    void initPreviewWidget();
    void initRightWidget();
    void addspaceritem();
    void initEditWidget();
    void initBuildWidget();
    void setThumbnailIcon(QString path);
    static bool g_themeChange;
    void startShowPlymouth();
    void eidtInitWidget(const HistoryInfo &InfoData);
    void refresh();
    void useMaximumScale();
    void useRestoreScale();
    void setBuildBtnText(bool isglobal);
signals:
    void newPlymouthFilePath(const QString& path);
    void refreshPlymouthDir();
    void build();
private:
    BasePreviewWidget *m_previewwidget;
    PlymouthImageWidget *m_preview;
    QWidget *m_rightwidget;
    QWidget *m_plymouthwidget;
    QWidget *m_buildwidget;
    QVBoxLayout *m_previewlayout;
    PlymouthCustomLabel *m_customLabel;
    QString m_imagedirpath;
    QRadioButton *pic;
    QLabel *pixLabel;
    QRadioButton *video;
    QLabel *videoLabel;
    QPushButton *m_buildButton;
};

#endif // PLYMOUTHMOUTHTHEMEWIDGET_H
