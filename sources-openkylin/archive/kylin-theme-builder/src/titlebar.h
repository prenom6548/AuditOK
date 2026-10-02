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
#ifndef TITLEBAR_H
#define TITLEBAR_H
#include "./module/aboutdialog.h"
#include "./module/savepathdialog.h"
#include "fileProcess/fileprocess.h"
#include <kysdk/diagnosetest/libkydatacollect.h>

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QToolButton>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);
    void toggleMaximize();
    QPushButton *m_gohomebtn = nullptr;

    void setGoHomeBtn(bool isable);
signals:
    void gohomesignal();
    void savePathUpdate(const QString& filePath);
    void maximizesignal();
    void restoresignal();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    bool m_ismaximized;
    QToolButton *m_optionbtn = nullptr; // 菜单选项
    QPushButton *m_minimumbtn = nullptr;
    QPushButton *m_maximumbtn = nullptr;
    QPushButton *recoverNormalSizeBtn = nullptr;
    QPushButton *m_closebtn = nullptr;
    SavePathDialog *m_savepathdlg;
    QGSettings*iconSettings = nullptr;
    bool isMaximizedDuringDrag() const;
    QIcon m_icon;
    QIcon scaledIcon;
    QLabel *iconlabel;
};

#endif // TITLEBAR_H
