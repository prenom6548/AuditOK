/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#ifndef DETECTPAGEWIDGET_H
#define DETECTPAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QIcon>
#include <QPalette>
#include <QSize>
#include <QTimer>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDir>
#include <QMovie>
#include <QGSettings>
#include <QDebug>
#include "globalsignal.h"
#include "svghandler.h"
#include "ktranslucentfloor.h"
#include <themeController.h>

#define DetectPageIconSize  QSize(40, 40)
#define UKUI_THEME_GSETTING_PATH "org.ukui.style"
#define FITCONTROLTRANS "org.ukui.control-center.personalise"

using namespace kdk;
class DetectPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DetectPageWidget(QWidget *parent = nullptr);

    void setupGui();
    void initConnect();
    void getFileListNum();
    QFileInfoList GetFileList(QString path);

public slots:
    void showWaitImages();
    void setWaitTimerStart();
    void setWaitTimerStop();
    void changeControlTransSlot(const QString &key);
    void transChange();
private:
    int num = 0;
    int count = 0;
    double m_tran = 0.60; //透明度

    QFileInfo fileinfo;
    QString path;
    QTimer *waitingTimer = nullptr;
    QLabel *m_detectPageIcon = nullptr;
    QLabel *m_detectPageText = nullptr;
    QString themeColor = "white";
    bool m_effect = false;
    bool isWayland = false;
    QGSettings *m_themeData = nullptr;
    QGSettings *m_pGsettingControlTrans = nullptr; //控制面板透明度
    QVBoxLayout *m_detectPageVLayout = nullptr;

    QColor m_backColor = QColor(255, 255, 255, 125);
    void paintEvent(QPaintEvent *event);            // 解决毛玻璃特效的问题
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // DETECTPAGEWIDGET_H
