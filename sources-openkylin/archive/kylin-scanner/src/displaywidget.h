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
#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QWidget>
#include <QStackedLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QEvent>
#include <QPalette>
#include <QIcon>
#include <QScrollArea>

#include "detectpagewidget.h"
#include "failedpagewidget.h"
#include "successpagewidget.h"
#include "include/common.h"
#include "include/theme.h"

class DisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DisplayWidget(QWidget *parent = nullptr);

    void setupGui();
    void initConnect();

Q_SIGNALS:
    void detectScanDevicesSignal();

public slots:
    void showDetectPageSlot();
    void showSuccessPageSlot(bool isSucceed);
    void showFailedPageSlot();

    void showSuccessImageHandlePageSlot();
    void refreshListSlot();
    void createAndExecuteScript();

private:
    DetectPageWidget *m_defaultConnectDetectPageWidget = nullptr;
    FailedPageWidget *m_defaultConnectFailedPageWidget = nullptr;
    SuccessPageWidget *m_defaultConnectSuccessPageWidget = nullptr;

    QStackedLayout *m_displayStackedLayout = nullptr;

};

#endif // DISPLAYWIDGET_H
