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
#ifndef LEFTIMAGEHANDLESUCCESSPAGEWIDGET_H
#define LEFTIMAGEHANDLESUCCESSPAGEWIDGET_H

#include "thumbnailwidget.h"
#include "showimagewidget.h"
//#include "toolbarwidget.h"
#include "showocrwidget.h"
#include "include/common.h"

#include <QWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>


class LeftImageHandleSuccessPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LeftImageHandleSuccessPageWidget(QWidget *parent = nullptr);

    void setupGui();
    void initConnect();

private:
    ThumbnailWidget *m_thumbnailWidget = nullptr;

    ShowImageWidget *m_showImageWidget = nullptr;
    showOcrWidget *m_showOcrWidget = nullptr;
    QStackedWidget *m_showImageOrOcrStackWidget = nullptr;
    QVBoxLayout *m_showImageAndToolBarVLayout = nullptr;

    QHBoxLayout *m_leftImageHandleSuccessPageHLayout = nullptr;


signals:

public slots:
    void showOcrWidgetSlot();
    void showImageWidgetSlot();
    void updatOcrText();
};

#endif // LEFTIMAGEHANDLESUCCESSPAGEWIDGET_H
