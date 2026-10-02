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
#ifndef LEFTSUCCESSPAGEWIDGET_H
#define LEFTSUCCESSPAGEWIDGET_H

#include "include/common.h"

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QPalette>
#include <QSize>



class LeftSuccessPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LeftSuccessPageWidget(QWidget *parent = nullptr);

    void setupGui();

signals:

private:
    QLabel *m_connectSuccessIcon = nullptr;
    QLabel *m_connectSuccessText = nullptr;
    QVBoxLayout *m_connectSuccessVLayout = nullptr;
    bool isDarkTheme();
private slots:
    void initTheme();
    void updateText(bool hasdevice);
};

#endif // LEFTSUCCESSPAGEWIDGET_H
