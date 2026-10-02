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
#ifndef SUCCESSPAGEWIDGET_H
#define SUCCESSPAGEWIDGET_H

#include "leftimagehandlesuccesspagewidget.h"
#include "leftsuccesspagewidget.h"
#include "scansettingswidget.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QStackedLayout>
#include <QScrollArea>

class SuccessPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SuccessPageWidget(QWidget *parent = nullptr);

    void setupGui();
    void initConnect();

Q_SIGNALS:
    void showImageHandlePageSignal();

public slots:
    void showLeftImageHandleSuccessPage();
    void updateScanSettingsSlot(bool isWork);


private:
    QStackedWidget *m_leftStackSuccessPageWidget = nullptr;
    ScanSettingsWidget *m_scanSettingsWidget = nullptr;
    QScrollArea *m_scanSettingsSrollArea = nullptr;
    LeftImageHandleSuccessPageWidget *m_leftImageHandleSuccessPageWidget = nullptr;
    LeftSuccessPageWidget *m_leftSuccessPageWidget = nullptr;
    QHBoxLayout *m_successPageHLayout = nullptr;

};

#endif // SUCCESSPAGEWIDGET_H
